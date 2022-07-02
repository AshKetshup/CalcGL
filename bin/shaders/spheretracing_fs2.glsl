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




float GetDist(vec3 p) {
    Sphere s1 = Sphere(
        vec4(0, 1, -6, 1),
        vec4(0, 1, 0, 1)
    );
    
    // float minSphereDist = length(p-spheres[nspheres].center.xyz)-spheres[nspheres].center.w;
    // float sphereDist;
    // objectColor = spheres[nspheres].color;
    // 
    // for (int i = 0; i < nspheres - 1; i++) {
    //     sphereDist = length(p-spheres[i].center.xyz)-spheres[i].center.w;
    //     if (sphereDist < minSphereDist) {
    //         minSphereDist = sphereDist;
    //         objectColor = spheres[i].color;
    //     }
    // }
    // float d = min(minSphereDist, planeDist);
    // if (planeDist < minSphereDist) {
    //     objectColor = vec4(1, 1, 1, 0);
    //     return planeDist;
    // }
    // return minSphereDist;

    float planeDist = p.y;

    float sphereDist = length(p-s1.center.xyz)-s1.center.w;
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