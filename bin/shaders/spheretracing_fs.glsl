#version 330 core

#define MAX_STEPS 100
#define MAX_DIST 100.
#define SURF_DIST .01

uniform vec2 iResolution;
uniform float iTime;

out vec4 fragColor;

float GetDist(vec3 p) {
    vec4 s = vec4(0, 1, 6, 1);
    
    float sphereDist =  length(p-s.xyz)-s.w;
    float planeDist = p.y;
    
    float d = min(sphereDist, planeDist);
    return d;
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
    vec3 lightPos = vec3(0, 5, 6);
    lightPos.xz += vec2(sin(iTime), cos(iTime))*2.;
    vec3 l = normalize(lightPos-p);
    vec3 n = GetNormal(p);
    
    float dif = clamp(dot(n, l), 0., 1.);
    float d = RayMarch(p+n*SURF_DIST*2., l);
    if(d<length(lightPos-p)) dif *= .1;
    
    return dif;
}

void main()
{
    vec2 uv = (gl_FragCoord.xy - .5 * iResolution.xy) / iResolution.y;

    vec3 col = vec3(0);
    
    vec3 ro = vec3(0, 1, 0);
    vec3 rd = normalize(vec3(uv.x, uv.y, 1));

    float d = RayMarch(ro, rd);
    
    vec3 p = ro + rd * d;
    
    float dif = GetLight(p);
    col = vec3(dif);
    
    col = pow(col, vec3(.4545));    // gamma correction
    
    fragColor = vec4(col, 1.0);
}





// #define MAX_STEPS 100
// #define SURF_DIST .01f
// #define INFINITY  100000.f
// 
// //#define SPHERE_AMNT 1;
// 
// out vec4 color;
// 
// uniform vec3  lightPos;
// uniform vec4  lightColor;
// uniform vec3  camPos;
// uniform vec3  camDirFront;
// uniform vec3  camDirUp;
// uniform vec3  camDirRight;
// uniform float camFOV;
// uniform float renderDistance;
// 
// uniform vec2  iResolution;
// 
// 
// // Sphere(center, radius);
// struct Sphere {
// 	vec3  center;
// 	float radius;
// };
// 
// float sphereDist(Sphere s, vec3 p) {
// 	return distance(s.center, p) - s.radius;
// }
// 
// 
// // Find a point in a Ray
// vec3 findPRay(vec3 rPos, vec3 rDir, float dist) {
// 	return (rPos + dist * rDir);
// }
// 
// // Find a point in the plain.
// vec3 findPPlain(vec3 pPos, vec3 pH, vec3 pV, vec2 coords) {
// 	return (pPos + coords.x * pH + coords.y * pV);
// }
// 
// 
// float getDist(vec3 p) {
// 	float resDist = INFINITY;
// 
// 	// <Spheres>
// 	Sphere s1 = Sphere(vec3(0.f, 1.f, 6.f), 1.f);
// 	resDist = min(sphereDist(s1, p), resDist);
// 
// 	return resDist;
// }
// 
// vec3 getNormal(vec3 p) {
// 	float d = getDist(p);
// 	vec2  e = vec2(SURF_DIST, 0.f);
// 
// 	vec3  n = d - vec3(
// 		getDist(p - e.xyy),
// 		getDist(p - e.yxy),
// 		getDist(p - e.yyx)
// 	);
// 
// 	return normalize(n);
// }
// 
// float rayMarch(vec3 ro, vec3 rd) {
// 	float dO = 0.;
// 
// 	for (int i = 0; i < MAX_STEPS; i++) {
// 		vec3 p = ro + rd * dO;
// 
// 		float dS = getDist(p);
// 		dO += dS;
// 
// 		if (dO > renderDistance || dS < SURF_DIST)
// 			break;
// 	}
// 
// 	return dO;
// }
// 
// float getLight(vec3 p) {
// 	vec3 l = normalize(lightPos - p);
// 	vec3 n = getNormal(p);
// 
// 	float dif = clamp(dot(n, l), 0.f, 1.f);
// 	float d = rayMarch(p + n * SURF_DIST * 2., l);
// 
// 	if (d < distance(lightPos, p))
// 		dif *= .1f;
// 
// 	return dif;
// }
// 
// void main() {
// 	vec2 uv = gl_FragCoord.xy - 0.5; // * iResolution.xy) / iResolution.y;
// 	
// 	vec3 ro = findPPlain(
// 		findPRay(camPos, normalize(camDirFront), camFOV),
// 		normalize(camDirRight),
// 		normalize(camDirUp),
// 		uv
// 	);
// 	vec3 rd = normalize(ro - camPos);
// 	
// 	float d = rayMarch(ro, rd);
// 	vec3 p  = findPRay(ro, rd, d);
// 	
// 	float dif   = getLight(p);
// 	vec3 fcolor = vec3(dif);
// 	color       = vec4(fcolor, 1.f);
// 	//color = lightColor;
// }