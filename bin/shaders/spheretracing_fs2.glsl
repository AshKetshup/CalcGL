#version 450 core

#define MAX_STEPS 100
#define SURF_DIST .01f
#define INFINITY  100000.f
#define MAX_DIST 300.

//#define SPHERE_AMNT 1;

uniform vec2 iResolution;
uniform float iTime;

out vec4 fragColor;

uniform vec3  lightPos;
uniform vec3  lightColor;
uniform vec3  camPos;
uniform vec3  camDirFront;
uniform vec3  camDirUp;
uniform vec3  camDirRight;
uniform float camFOV;
uniform float renderDistance;

uniform int nspheres;

vec4 objectColor;

// Sphere(center, color);
struct Sphere {
	vec4  center;   // includes radius on w
    vec4  color;
};

// Uniform block named InstanceBlock, follows std140 alignment rules
layout (std140, binding = 0) uniform uboSpheres {
  Sphere spheres[100];
};


// Find a point in a Ray
vec3 findPRay(vec3 rPos, vec3 rDir, float dist) {
	return (rPos + dist * rDir);
}

// Find a point in the plain.
vec3 findPPlain(vec3 pPos, vec3 pH, vec3 pV, vec2 coords) {
	return (pPos + coords.x * pH + coords.y * pV);
}


/* Signed Distance Functions */
float sdPlane( vec3 p, vec3 n, float h ) {
    // n must be normalized
    n = normalize(n);
    
    return dot(p, n) + h;
}

float sdSphere(vec3 p, vec4 s ) {
    p -= s.xyz;
    return length(p) - s.w;
}

float sdCapsule(vec3 p, vec3 a, vec3 b, float r) {
    vec3 ab = b - a;
    vec3 ap = p - a;

    float t = dot(ab, ap) / dot(ab, ab);
    t = clamp(t, 0., 1.);

    vec3 c = findPRay(a, ab, t);
    
    return distance(p, c) - r;
}

float sdTorus(vec3 p, vec3 center, vec2 r) {
    p -= center;
    float x = length(p.xz) - r.x;

    return length(vec2(x, p.y)) - r.y;
}

float sdBox( vec3 p, vec3 center, vec3 b ) {
    p -= center;

    vec3 q = abs(p) - b;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdBoxFrame( vec3 p, vec3 center, vec3 dim, float thickness ) {
    p -= center;
    
    p = abs(p) - dim;
    vec3 q = abs(p + thickness) - thickness;
    return min(min(
        length(max(vec3(p.x, q.y, q.z), 0.0)) + min(max(p.x, max(q.y, q.z)), 0.0),
        length(max(vec3(q.x, p.y, q.z), 0.0)) + min(max(q.x, max(p.y, q.z)), 0.0)),
        length(max(vec3(q.x, q.y, p.z), 0.0)) + min(max(q.x, max(q.y, p.z)), 0.0)
    );
}

float sdCylinder(vec3 p, vec3 a, vec3 b, float r) {
    vec3  ba = b - a;
    vec3  pa = p - a;
    float baba = dot(ba,ba);
    float paba = dot(pa,ba);
    float x = length(pa*baba-ba*paba) - r*baba;
    float y = abs(paba-baba*0.5)-baba*0.5;
    float x2 = x*x;
    float y2 = y*y*baba;
    
    float d = (max(x,y)<0.0)?-min(x2,y2):(((x>0.0)?x2:0.0)+((y>0.0)?y2:0.0));
    
    return sign(d)*sqrt(abs(d))/baba;
}

float sdCappedCone(vec3 p, vec3 a, vec3 b, float ra, float rb) {
    float rba  = rb-ra;
    float baba = dot(b-a,b-a);
    float papa = dot(p-a,p-a);
    float paba = dot(p-a,b-a)/baba;
    float x = sqrt( papa - paba*paba*baba );
    float cax = max(0.0,x-((paba<0.5)?ra:rb));
    float cay = abs(paba-0.5)-0.5;
    float k = rba*rba + baba;
    float f = clamp( (rba*(x-ra)+paba*baba)/k, 0.0, 1.0 );
    float cbx = x-ra - f*rba;
    float cby = paba - f;
    float s = (cbx<0.0 && cay<0.0) ? -1.0 : 1.0;
    return s*sqrt( min(cax*cax + cay*cay*baba,
                       cbx*cbx + cby*cby*baba) );
}

float sdHexPrism( vec3 p, vec3 center, vec2 h ) {
    p -= center;
  
    const vec3 k = vec3(-0.8660254, 0.5, 0.57735);
    p = abs(p);
    p.xy -= 2.0*min(dot(k.xy, p.xy), 0.0)*k.xy;
    vec2 d = vec2(
         length(p.xy-vec2(clamp(p.x,-k.z*h.x,k.z*h.x), h.x))*sign(p.y-h.x),
         p.z-h.y );
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float sdTriPrism( vec3 p, vec3 center, vec2 h ) {
    p -= center;
    
    vec3 q = abs(p);
    return max(q.z-h.y,max(q.x*0.866025+p.y*0.5,-p.y)-h.x*0.5);
}


float opOnion( in float sdf, in float thickness ) { return abs(sdf)-thickness; }

float opUnion( float d1, float d2 ) { return min(d1,d2); }
float opSubtraction( float d1, float d2 ) { return max(-d1,d2); }
float opIntersection( float d1, float d2 ) { return max(d1,d2); }


float opSmoothUnion( float d1, float d2, float k ) {
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h); 
}

float opSmoothSubtraction( float d1, float d2, float k ) {
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h); 
}

float opSmoothIntersection( float d1, float d2, float k ) {
    float h = clamp( 0.5 - 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) + k*h*(1.0-h); 
}

float roundEffect(float sdf, float r) {
    return sdf - r;
}

float GetDist(vec3 p) {
    Sphere s1 = Sphere(
        vec4(0., 1., -6., 1.),
        vec4(0., 1., 0., 1.)
    );

    float dist = INFINITY;
    vec4 s = vec4(vec3(0., 3., 0.), 1.5);

    dist = min(dist, sdPlane(p, vec3(0., 1., 0.), 2.));
    dist = min(dist, sdSphere(p, s));
    dist = min(dist, sdTorus(p, s.xyz, vec2(s.w * 2., s.w/5.)));
    
    dist = min(dist, sdBox(p, vec3(7., 3., 0.), vec3(s.w)));
    dist = min(dist, sdBoxFrame(p, vec3(7., 3., 0.), vec3(s.w + 0.5), .1));
    
    dist = min(dist, sdCapsule(p, vec3(-7., 2. , 0.), vec3(-7., 4., 0.), s.w));
    
    dist = min(dist, sdCylinder(p, vec3(-7., 1., -7.), vec3(-7., 5., -7.), s.w));

    dist = min(dist, sdCappedCone(p, vec3(7., 1., 7.), vec3(7., 5., 7.), s.w, 0));

    dist = min(dist, sdHexPrism(p, vec3(-7., 3., 7.), s.ww));

    dist = min(dist, sdTriPrism(p, vec3(0., 3., 7.), s.ww));

    dist = min(dist, opSmoothSubtraction(sdSphere(p, vec4(vec3(0., 3.5, -7.), s.w*0.9)), sdSphere(p, vec4(vec3(0.5, 3., -7.5), s.w)), 0.3));


    return roundEffect(dist, cos(iTime));
    return dist;
}

float RayMarch(vec3 ro, vec3 rd) {
    float dO=0.;
    
    for(int i=0; i<MAX_STEPS; i++) {
        vec3 p = ro + rd*dO;
        float dS = GetDist(p);
        dO += dS;
        if(dO>MAX_DIST || dS<SURF_DIST) break;
    }
    
    return dO;
}

vec3 GetNormal(vec3 p) {
    float d = GetDist(p);
    vec2 e = vec2(.01, 0);
    
    vec3 n = d - vec3(
        GetDist(p-e.xyy),
        GetDist(p-e.yxy),
        GetDist(p-e.yyx)
    );
    
    return normalize(n);
}

float GetLight(vec3 p, vec3 lightPos) {
    vec3 l = normalize(lightPos-p);
    vec3 n = GetNormal(p);
    
    float dif = clamp(dot(n, l), 0., 1.);
    float d = RayMarch(p + n * SURF_DIST * 2., l);
    if(d < length(lightPos-p)) 
        dif *= .1;
    
    return dif;
}

void main()
{
    vec2 uv = (gl_FragCoord.xy - .5 * iResolution.xy) / iResolution.y;

    vec3 col = vec3(0);
    
    // Version Igor
    // vec3 ro = findPPlain(
	// 	findPRay(camPos, normalize(camDirFront), camFOV),
	// 	normalize(camDirRight),
	// 	normalize(camDirUp),
	// 	uv
	// );
    // vec3 rd = normalize(ro - camPos);

    // Version Ash
    vec3 ro = camPos;
    vec3 rd = normalize(
        findPPlain(
		    findPRay(camPos, normalize(camDirFront), camFOV),
		    normalize(camDirRight),
		    normalize(camDirUp),
		    uv
	    ) - ro
    );

    float d = RayMarch(ro, rd);
    
    // vec3 p = ro + rd * d;
    vec3 p = findPRay(ro, rd, d);
    
    vec3 light[] = {
        lightPos,
        //vec3(0., 15., 0.),
        vec3(15. + cos(iTime), 15. + cos(iTime), 3. + cos(iTime))
    };

    float dif = 0.;
    for (int i = 0; i < 2; i++)
        dif += GetLight(p, light[i]);

    dif = clamp(dif, 0., 1.);
    
    col = objectColor.xyz + lightColor.xyz * vec3(dif);
    col = pow(col, vec3(.4545));    // gamma correction
    
    fragColor = vec4(col, 1.0);
}