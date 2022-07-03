#version 450 core

#define MAX_STEPS 100
#define SURF_DIST .01f
#define INFINITY  100000.f
#define MAX_DIST 100.

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

float sdBox(vec3 p, vec3 center, vec3 dim) {
    p -= center;

    return length(max(abs(p)-dim, 0.));
}

float GetDist(vec3 p) {
    Sphere s1 = Sphere(
        vec4(0, 1, -6, 1),
        vec4(0, 1, 0, 1)
    );

    float dist = INFINITY;

    float planeDist = p.y;
    dist = min(dist, planeDist);

    float sphereDist = length(p-s1.center.xyz)-s1.center.w;
    dist = min(dist, sphereDist);
    
    dist = min(dist, sdTorus(p, s1.center.xyz, vec2(s1.center.w*2. , s1.center.w/5.)));

    dist = min(dist, sdBox(p, vec3(0., 1., 0.), vec3(s1.center.w)));
    
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
        GetDist(p-e.yyx));
    
    return normalize(n);
}

float GetLight(vec3 p) {
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
    
    float dif = GetLight(p);
    
    col = lightColor.xyz * vec3(dif);
    col = pow(col, vec3(.4545));    // gamma correction
    
    fragColor = vec4(col, 1.0);
}