#version 330 core

uniform vec3  lightPos;
uniform vec4  lightColor;
//uniform vec4  colorAttempt;
uniform vec3  camPos;
uniform vec3  camDirFront;
uniform vec3  camDirUp;
uniform vec3  camDirRight;
uniform float camFOV;
uniform float renderDistance;

out vec4 color;

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

// Find a point in a Ray
vec3 findPRay(vec3 rPos, vec3 rDir, float dist) {
    return (rPos + dist * rDir);
}

// Find a point in the plain.
vec3 findPPlain(vec3 pPos, vec3 pH, vec3 pV, vec2 coords) {
    return (pPos + coords.x * pH + coords.y * pV);
}

vec3 bisection(vec3 leftP, vec3 rightP, float threshold) {
    while (true) {
        vec3  midPoint       = (leftP + rightP)/2.f;
        float signedDistance = evalImplicitFunc(midPoint);
        
        if (abs(signedDistance) < threshold)
            return midPoint;
            
        if (signedDistance < 0.f)
            leftP  = midPoint;
        else
            rightP = midPoint;
    }
}

bool rayMarch(vec3 posStart, vec3 dir, vec3 dirR, vec3 dirU, float FOV, float aStep, float mDist, float mThreshold, vec2 coords) {
    // Get Point from generated Plain
    vec3 pointToDir = findPPlain(
        findPRay(posStart, dir, FOV), 
        dirR, 
        dirU, 
        coords
    );
    
    float t           = 0.f;
    vec3  marchingDir = normalize(pointToDir - posStart);
    vec3  leftPoint   = pointToDir;
    vec3  rightPoint  = vec3(0.f);
    bool  intercept   = false;
    bool  currentSign = (evalImplicitFunc(leftPoint) < 0.f);

    while (t < mDist || !intercept) {
        leftPoint = findPRay(pointToDir, marchingDir, t);
        t += aStep;
        rightPoint = findPRay(pointToDir, marchingDir, t);
        
        intercept = (evalImplicitFunc(rightPoint) < 0.f) != currentSign;
    }
    
    if (t < mDist)
        return false;
    
    vec3 pIntersected = bisection(leftPoint, rightPoint, mThreshold);
    return pIntersected != vec3(0.f);
}


void main() {
    // Since gl_PointCoord is [0, 1] then we need  
    // to shift them in half to get a centered origin
    vec2 coords = gl_PointCoord - vec2(.5f);

    // Normalize camera vectors
    vec3 cFront = normalize(camDirFront);
    vec3 cRight = normalize(camDirRight);
    vec3 cUp    = normalize(camDirUp);

    // Ray variables
    // Ray Origin
    vec3 rPos   = camPos;
    // Ray Direction
    vec3 rDir   = cFront;

    // Plain variables
    // Plain central point
    vec3 pPos   = rPos + camFOV * rDir;
    // Plain y vector
    vec3 pV     = cUp;
    // Plain x vector
    vec3 pH     = cRight;
    // Plain calculated point from screen fragment coordinates
    vec3 start  = findPPlain(pPos, pH, pV, coords);

    // Ray for the algorithm
    // RayMarching starting point
    vec3 mRPos  = start;
    // RayMarching direction
    vec3 mRDir  = normalize(start - camPos);
    
    // Declaration algoStep, maxDist
    float algoStep      = 0.1f;
    float algoThreshold = 0.0001f;
    float maxDist       = renderDistance;

    // Initiate RayMarch
    bool isHit = rayMarch(camPos, cFront, cRight, cUp, camFOV, algoStep, maxDist, algoThreshold, coords);

    // Output to screen
    color = isHit ? vec4(1.f,1.f,1.f,1.0) : vec4(0.f,0.f,0.f,1.0);
    //color = colorAttempt;
}