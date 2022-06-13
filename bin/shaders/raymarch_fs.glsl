#version 330 core

in vec3  lightPos;
in vec3  lightColor;
in vec3  camPos;
in vec3  camDirFront;
in vec3  camDirUp;
in vec3  camDirRight;
in float camFOV;
in float rMMaxDist;

out vec4 color;

// Defines a implicit function
float iFunction(in vec3 p) {
    float x = p.x;
    float y = p.y;
    float z = p.z;

    return pow(x, 2) + pow(y, 2) + pow(z, 2) + 3.f;
    // return <iFunction>;
}

// Finds if the implicit function is intercepted by a segment of 2 points
bool isIFunctionIntercepted(in vec3 pA, in vec3 pB) {
    float value;

    value = iFunction(pA);
    bool pASign = value > 0.f;

    value = iFunction(pB);
    bool pBSign = value > 0.f;

    return !((pASign && pBSign) || (!pASign && !pBSign));
}

// Find a point in a Ray
vec3 findPRay(vec3 rPos, vec3 rDir, float dist) {
    return (rPos + dist * rDir);
}

// Find a point in the plain.
vec3 findPPlain(vec3 pPos, vec3 pH, vec3 pV, vec2 coords) {
    return (pPos + coords.x * pH + coords.y * pV);
}

// Ray march from a point into a direction.
vec3 rayMarch(vec3 rPos, vec3 rDir, float maxDist, float p, float stepSize) {
    float stepAux = stepSize;
    float aux = 0.f;
    vec3 point = rPos;

    bool isIntercepted = isIFunctionIntercepted(rPos, point);
    while (!isIntercepted && aux < maxDist) {
        aux += stepSize;
        point = findPRay(rPos, rDir, aux);
    }

    if (aux >= maxDist)
        return vec3(0);

    while (true) {
        aux -= (stepAux /= 2.f);
        vec3 midPoint = findPRay(rPos, rDir, aux);

        if (stepAux < p) {
            point = midPoint;
            break;
        }

        if (!isIFunctionIntercepted(rPos, midPoint))
            aux += (stepAux /= 2.f);
    }

    return point;
}

void main() {
    vec3 cFront = normalize(camDirFront);
    vec3 cRight = normalize(camDirRight);
    vec3 cUp    = normalize(camDirUp);

    // Ray
    vec3 rPos = camPos;
    vec3 rDir = cFront;

    // Plain
    vec3 pPos = rPos + camFOV * rDir;
    vec3 pV   = cUp;
    vec3 pH   = cRight;
    vec3 start = findPPlain(pPos, pH, pV, gl_PointCoord);

    // Ray for the algorithm
    vec3 mRPos = start;
    vec3 mRDir = normalize(start - camPos);

    // If RayMarching hits something then paint it as black
    vec3 foundP = rayMarch(mRPos, mRDir, rMMaxDist, .01f, .01f);
    vec3 lightDir = normalize(lightPos - foundP);
    
    // If (is an object between the point and light) : 
    //     is painted as black
    // Else : 
    //     paints with a gray (darker the further it is from the light)
    if (rayMarch(foundP, lightDir, rMMaxDist, .01f, .01f) != vec3(0))
        color = vec4(0.f, 0.f, 0.f, 1.f);
    else
        color = vec4(vec3(distance(camPos, foundP) / rMMaxDist), 1.f);
}