//------------------------------------------------------------------------------
//
// CalcGL
//
// RUNTIME LIBRARIES PACKAGE
//    glslcode.h
//
// DESCRIPTION:
// -----------
// Contains the shaders hard-coded in order to provide a failsafe option in case
// of the original files being missing.
// 
//------------------------------------------------------------------------------

#ifndef GLSLCODE_H
#define GLSLCODE_H

const char rayMarch_fragment_shader[] =
"#version 330 core\n"
"\n"
"uniform vec3  lightPos;\n"
"uniform vec4  lightColor;\n"
"uniform vec3  camPos;\n"
"uniform vec3  camDirFront;\n"
"uniform vec3  camDirUp;\n"
"uniform vec3  camDirRight;\n"
"uniform float camFOV;\n"
"uniform float renderDistance;\n"
"\n"
"out vec4 color;\n"
"\n"
"bool even(in int n) {\n"
"    return n % 2 == 0;\n"
"}\n"
"\n"
"// Defines a implicit function\n"
"float evalImplicitFunc(in vec3 point) {\n"
"    float x = point.x;\n"
"    float y = point.y;\n"
"    float z = point.z;\n"
"\n"
"    int count = 0;\n"
"    float prod = 1.f;\n"
"    float image = 0.f;\n"
"\n"
"    // <gamma conditions>\n"
"\n"
"    return prod * (even(count) ? -1.f : 1.f);\n"
"}\n"
"\n"
"// Find a point in a Ray\n"
"vec3 findPRay(vec3 rPos, vec3 rDir, float dist) {\n"
"    return (rPos + dist * rDir);\n"
"}\n"
"\n"
"// Find a point in the plain.\n"
"vec3 findPPlain(vec3 pPos, vec3 pH, vec3 pV, vec2 coords) {\n"
"    return (pPos + coords.x * pH + coords.y * pV);\n"
"}\n"
"\n"
"vec3 bisection(vec3 leftP, vec3 rightP, float threshold) {\n"
"    while (true) {\n"
"        vec3  midPoint = (leftP + rightP) / 2.f;\n"
"        float signedDistance = evalImplicitFunc(midPoint);\n"
"\n"
"        if (abs(signedDistance) < threshold)\n"
"            return midPoint;\n"
"\n"
"        if (signedDistance < 0.f)\n"
"            leftP = midPoint;\n"
"        else\n"
"            rightP = midPoint;\n"
"    }\n"
"}\n"
"\n"
"bool rayMarch(vec3 posStart, vec3 dir, vec3 dirR, vec3 dirU, float FOV, float aStep, float mDist, float mThreshold, vec2 coords) {\n"
"    // Get Point from generated Plain\n"
"    vec3 pointToDir = findPPlain(\n"
"        findPRay(posStart, dir, FOV),\n"
"        dirR,\n"
"        dirU,\n"
"        coords\n"
"    );\n"
"\n"
"    float t = 0.f;\n"
"    vec3  marchingDir = normalize(pointToDir - posStart);\n"
"    vec3  leftPoint = pointToDir;\n"
"    vec3  rightPoint = vec3(0.f);\n"
"    bool  intercept = false;\n"
"    bool  currentSign = (evalImplicitFunc(leftPoint) < 0.f);\n"
"\n"
"    while (t < mDist || !intercept) {\n"
"        leftPoint = findPRay(pointToDir, marchingDir, t);\n"
"        t += aStep;\n"
"        rightPoint = findPRay(pointToDir, marchingDir, t);\n"
"\n"
"        intercept = (evalImplicitFunc(rightPoint) < 0.f) != currentSign;\n"
"    }\n"
"\n"
"    if (t < mDist)\n"
"        return false;\n"
"\n"
"    vec3 pIntersected = bisection(leftPoint, rightPoint, mThreshold);\n"
"    return pIntersected != vec3(0.f);\n"
"}\n"
"\n"
"\n"
"void main() {\n"
"    // Since gl_PointCoord is [0, 1] then we need  \n"
"    // to shift them in half to get a centered origin\n"
"    vec2 coords = gl_PointCoord - vec2(.5f);\n"
"\n"
"    // Normalize camera vectors\n"
"    vec3 cFront = normalize(camDirFront);\n"
"    vec3 cRight = normalize(camDirRight);\n"
"    vec3 cUp = normalize(camDirUp);\n"
"\n"
"    // Ray variables\n"
"    // Ray Origin\n"
"    vec3 rPos = camPos;\n"
"    // Ray Direction\n"
"    vec3 rDir = cFront;\n"
"\n"
"    // Plain variables\n"
"    // Plain central point\n"
"    vec3 pPos = rPos + camFOV * rDir;\n"
"    // Plain y vector\n"
"    vec3 pV = cUp;\n"
"    // Plain x vector\n"
"    vec3 pH = cRight;\n"
"    // Plain calculated point from screen fragment coordinates\n"
"    vec3 start = findPPlain(pPos, pH, pV, coords);\n"
"\n"
"    // Ray for the algorithm\n"
"    // RayMarching starting point\n"
"    vec3 mRPos = start;\n"
"    // RayMarching direction\n"
"    vec3 mRDir = normalize(start - camPos);\n"
"\n"
"    // Declaration algoStep, maxDist\n"
"    float algoStep = 0.1f;\n"
"    float algoThreshold = 0.0001f;\n"
"    float maxDist = renderDistance;\n"
"\n"
"    // Initiate RayMarch\n"
"    bool isHit = rayMarch(camPos, cFront, cRight, cUp, camFOV, algoStep, maxDist, algoThreshold, coords);\n"
"\n"
"    // Output to screen\n"
"    color = isHit ? vec4(1.f, 1.f, 1.f, 1.0) : vec4(0.f, 0.f, 0.f, 1.0);\n"
"}";

const char surface_fragment_shader[] =
"#version 330 core\n"
"\n"
"out vec4 color;\n"
"\n"
"uniform sampler2D u_texture;\n"
"uniform vec2	  iResolution;\n"
"\n"
"void main() {\n"
"	vec2 uv = gl_PointCoord / iResolution - vec2(.5);\n"
"\n"
"	color = texture(u_texture, uv);\n"
"}\n";

const char surface_vertex_shader[] =
"#version 330 core\n"
"\n"
"const vec2 quad_vertices[4] = vec2[4](vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0));\n"
"\n"
"void main() {\n"
"    gl_Position = vec4(quad_vertices[gl_VertexID], 0.0, 1.0);\n"
"}\n";


const char font_fragment_shader[] =
"#version 330 core\n"
"in vec2 TexCoords;\n"
"out vec4 color;\n"
"\n"
"uniform sampler2D text;\n"
"uniform vec3 textColor;\n"
"\n"
"void main() {\n"
"    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
"    color = vec4(textColor, 1.0) * sampled;\n"
"}";

const char font_vertex_shader[] =
"#version 330 core\n"
"layout (location = 0) in vec4 vertex;\n"
"out vec2 TexCoords;\n"
"\n"
"uniform mat4 projection;\n"
"\n"
"void main() {\n"
"    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
"    TexCoords = vertex.zw;\n"
"}";

const char logo_vertex_shader[] =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"layout (location = 2) in vec2 aTexCoord;\n"
"\n"
"uniform mat4 projection;\n"
"\n"
"out vec3 ourColor;\n"
"out vec2 TexCoord;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = projection * vec4(aPos, 1.0);\n"
"    ourColor = aColor;\n"
"    TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
"}";

const char logo_fragment_shader[] =
"#version 330 core\n"
"out vec4 FragColor;\n"
"\n"
"in vec3 ourColor;\n"
"in vec2 TexCoord;\n"
"\n"
"uniform sampler2D ourTexture;\n"
"\n"
"void main()\n"
"{\n"
"    FragColor = texture(ourTexture, TexCoord);\n"
"}";

#define VERTEX      0b00000001              // Vertex shader
#define FRAGMENT    0b00000010              // Fragment shader
#define GEOMETRY    0b00000100              // Geometry shader (for future use)
#define SURFACE     0b00100000              // Surface shaders
#define FONT        0b01000000              // Font shaders
#define LOGO        0b10000000              // Logo shaders

#define RAYMARCH_VS "raymarch_vs.glsl"      // Surface vertex shader file name
#define RAYMARCH_FS "raymarch_fs.glsl"      // Surface fragment shader file name
#define SURF_VS     "pisurf_vs.glsl"        // Surface vertex shader file name
#define SURF_FS     "pisurf_fs.glsl"        // Surface fragment shader file name
#define FONT_FS     "font_fs.glsl"          // Font fragment shader file name
#define FONT_VS     "font_vs.glsl"          // Font vertex shader file name
#define LOGO_FS     "logo_fs.glsl"          // Logo fragment shader file name
#define LOGO_VS     "logo_vs.glsl"          // Logo vertex shader file name

#include <string>

// Gets name of shader from the int code
std::string nameOfShader(int);

// Checks if a given shader exists
bool shaderExists(std::string);

// Creates the shader, if not available, given the int code
bool createShader(std::string, int);

// Proceeds to automatically check and create missing shaders.
// Returns the number of shaders that were missing and were successfully corrected.
int autoCorrectShaders(std::string);

#endif