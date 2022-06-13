#version 330 core
#extension GL_ARB_separate_shader_objects: enable

// layout (location = 0) in vec3  aPos;
// layout (location = 1) in vec3  aNormal;

layout (location = 0) in vec3  lPos;
layout (location = 1) in vec3  lColor;
layout (location = 2) in vec3  cPos;
layout (location = 3) in vec3  cDirFront;
layout (location = 4) in vec3  cDirUp;
layout (location = 5) in vec3  cDirRight;
layout (location = 6) in float cFOV;
layout (location = 7) in float rayMaxDist;

out vec3  lightPos;
out vec3  lightColor;
out vec3  camPos;
out vec3  camDirFront;
out vec3  camDirUp;
out vec3  camDirRight;
out float camFOV;
out float rMMaxDist;
out float rMStepSize;

// out vec3 FragPos;
// out vec3 Normal;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;

void main() {
    // FragPos = vec3(model * vec4(aPos, 1.0));
    // FragPos.z = -FragPos.z;
    // Normal = mat3(transpose(inverse(model))) * aNormal;
    // Normal.z = -Normal.z;
    
    // gl_Position = projection * view * vec4(FragPos, 1.0);
    // gl_Position.z = -gl_Position.z;

    lightPos    = lPos;
    lightColor  = lColor;
    camPos      = cPos;
    camDirFront = cDirFront;
    camDirUp    = cDirUp;
    camDirRight = cDirRight;
    camFOV      = cFOV;
    rMMaxDist   = rayMaxDist;
}