#version 450 core

#define SURF_DIST .01

uniform vec2  iResolution;
uniform float iTime;

uniform vec3  lightPos;
uniform vec3  lightColor;

uniform vec3  fgColor;
uniform vec3  bgColor;

uniform vec3  camPos;
uniform vec3  camDirFront;
uniform vec3  camDirUp;
uniform vec3  camDirRight;
uniform float camFOV;

uniform float renderDistance;

out vec4 color;


/* GLOBAL VARIABLES */
vec3 point;
vec3 normal  = vec3(0.);
vec2 e       = vec2(.01, 0.);
float result = 0.f;
float curr;
float ro_f;


// Defines a implicit function
float evalImplicitFunc(vec3 point) {
    float x = point.x;
    float y = point.y;
    float z = point.z;
    
    float prod = 1.f;
    
    // <gamma conditions>

    return prod;
}



float RayMarch(vec3 ro, vec3 rd) {
    float dO = 0.;

    while (dO < renderDistance) {
        point = ro + rd * dO;
        
        curr = evalImplicitFunc(point);
        if ((ro_f <= 0. && curr >= 0.) || (ro_f >= 0. && curr <= 0.)) {
            normal = normalize(
                curr - vec3(
                    evalImplicitFunc(point - e.xyy), 
                    evalImplicitFunc(point - e.yxy), 
                    evalImplicitFunc(point - e.yyx)
                )
            );

            break;
        }
        
        dO += SURF_DIST;
    }
    
    return dO;
}

void main() {
    // Since gl_PointCoord is [0, 1] then we need  
    // to shift them in half to get a centered origin
    vec2 uv = (gl_FragCoord.xy - .5 * iResolution.xy) / iResolution.y;

    // Normalizes all Camera directions
    vec3 camDirF = normalize(camDirFront);
    vec3 camDirU = normalize(camDirUp   );
    vec3 camDirR = normalize(camDirRight);

    // Initializes color to the color of the background
    vec3 col     = bgColor;

    // Initializes algorithm starting point and direction
    vec3 ro = camPos;
    vec3 rd = normalize((
            (ro + camDirF * camFOV) +
		    (uv.x * camDirR) +
		    (uv.y * camDirU)
        ) - ro
    );

    // Evaluates the starting point
    ro_f = evalImplicitFunc(ro);

    // Gets the distance to an intercection
    float d = RayMarch(ro, rd);

    if (d < renderDistance) {
        // If the distance is smaller than the render distance 
        // we may change the color based on the light and foreground 
        // color

        // finds the intercepted point
        vec3 p = ro + rd * d;
        
        /* Calculates Light */
        vec3 l = normalize(lightPos - p);
        d = RayMarch(p + normal * SURF_DIST * 2., l);

        float dif = clamp(dot(normal, l), 0., 1.);
        if (d < length(lightPos - p))
            dif *= .1;
        
        // defines the output color
        col = fgColor * pow(lightColor * dif, vec3(.4545));
    }

    color = vec4(col, 1.);
}