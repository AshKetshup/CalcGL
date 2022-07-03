#version 450 core

#define SURF_DIST .01

uniform vec2 iResolution;
uniform float iTime;

uniform vec3  lightPos;
uniform vec4  lightColor;

uniform vec3  objectColor;

uniform vec3  camPos;
uniform vec3  camDirFront;
uniform vec3  camDirUp;
uniform vec3  camDirRight;
uniform float camFOV;

uniform float renderDistance;

out vec4 color;

vec3 normal = vec3(0.);

bool even(in int n) {
    return n % 2 == 0;
}

// Defines a implicit function
float evalImplicitFunc(in vec3 point) {
    float x = point.x;
    float y = point.y;
    float z = point.z;
    
    int count = 0;
    float prod = 1.f;
    float image = 0.f;
    
    // <gamma conditions>

    return prod * (even(count) ? -1.f : 1.f);
}

bool collatz(float x, float y) {
    return ((x >= 0.) && (y <= 0.)) || ((x <= 0.) && (y >= 0.));
}


// Find a point in a Ray
vec3 findPRay(vec3 rPos, vec3 rDir, float dist) {
    return (rPos + dist * rDir);
}

// Find a point in the plain.
vec3 findPPlain(vec3 pPos, vec3 pH, vec3 pV, vec2 coords) {
    return (pPos + coords.x * pH + coords.y * pV);
}


float ro_sign;

float rayMarch(vec3 ro, vec3 rd) {
    float curr = evalImplicitFunc(ro);
    vec2 e = vec2(.01, 0);
    vec3 p;
    
    float dO = 0.f;
    while (dO < renderDistance) {
        p = findPRay(ro, rd, dO);

        curr = evalImplicitFunc(p);
        if (collatz(ro_sign, curr)) {
            normal = normalize(
                curr - vec3(
                    evalImplicitFunc(p - e.xyy),
                    evalImplicitFunc(p - e.yxy),
                    evalImplicitFunc(p - e.yyx)
                )
            );

            break;
        }
        
        dO += SURF_DIST;
    }

    return dO;
}

float getLight(vec3 p) {
    vec3  l = normalize(lightPos - p);
    float d = rayMarch(findPRay(p, normal, SURF_DIST * 2.f), l);

    float dif = clamp(dot(normal, l), 0.f, 1.f);
    if (d < length(lightPos - p))
        dif *= .1;

    return dif;
}

void main() {
    // Since gl_PointCoord is [0, 1] then we need  
    // to shift them in half to get a centered origin
    vec2 uv = (gl_FragCoord.xy - .5 * iResolution.xy) / iResolution.y;

    vec3 ro = camPos;
    vec3 rd = normalize(
        findPPlain(
		    findPRay(camPos, normalize(camDirFront), camFOV),
		    normalize(camDirRight),
		    normalize(camDirUp),
		    uv
	    ) - ro
    );

    ro_sign = evalImplicitFunc(ro);
    float d = rayMarch(ro, rd);

    if (d < renderDistance) {
        vec3 p = findPRay(ro, rd, d);
        vec3 dif = vec3(getLight(p));

        vec3 lColor = lightColor.xyz * pow(dif, vec3(.4545));

        color = vec4(objectColor + lColor, 1.f);
    } else {
        color = vec4(vec3(0.f), 1.f);
    }
}