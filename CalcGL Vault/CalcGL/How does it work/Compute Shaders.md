[Compute Shaders in Modern OpenGL](https://www.youtube.com/watch?v=nF4X9BIUzx0)

# Compute Shaders in OpenGL
---

## What they are used for
Compute Shaders são independentes da pipeline de renderização pelo que são usados apenas para processamento paralelo em algo.

* Como são independentes da pipeline, não possuem qualquer **user input** ou **output**

## How they work
Setup of Compute Shader
```cpp
const char* computeShaderSource[] = { ... };

GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
glShaderSource(computeShader, 1, &computeShaderSource, NULL);
glCompileShader(computeShader);

GLuint computeProgram = glCreateProgram();
glAttachShader(computeProgram, computeShader);
glLinkProgram(computeProgram);
```

Running the Compute Shader
```cpp
glUseProgram(computeProgram);
glDispatchCompute(SCREEN_WIDTH, SCREEN_HEIGHT, 1);
glMemoryBarrier(GL_ALL_BARRIERS_BITS); // This might not be the best option
```
	Outras opções:
```Cpp
GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
GL_ELEMENT_ARRAY_BARRIER_BIT
GL_UNIFORM_BARRIER_BIT
GL_TEXTURE_FETCH_BARRIER_BIT
GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
GL_COMMAND_BARRIER_BIT
GL_PIXEL_BUFFER_BARRIER_BIT
GL_TEXTURE_UPDATE_BARRIER_BIT
GL_BUFFER_UPDATE_BARRIER_BIT
GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT
GL_FRAMEBUFFER_BARRIER_BIT
GL_TRANSFORM_FEEDBACK_BARRIER_BIT
GL_ATOMIC_COUNTER_BARRIER_BIT
GL_SHADER_STORAGE_BARRIER_BIT
GL_QUERY_BUFFER_BARRIER_BIT
```

Visto que compute shaders não conseguem renderizar precisaremos de ter a certeza que temos uma textura do tamanho da janela.

```cpp
GLuint screenTex;
glCreateTexture(GL_TEXTURE_2D, 1, &screenTex);
glTextureParameteri(screenTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTextureParameteri(screenTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTextureParameteri(screenTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTextureParameteri(screenTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTextureStorage2D(screenTex, 1, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT);
glBindImageTexture(0, screenTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
```

E 2 triangulos para renderizar pelo vertex shader

```cpp
GLfloat vertices[] = {
	-1.f, -1.f,  0.f,  0.f,  0.f, 
	-1.f,  1.f,  0.f,  0.f,  1.f,
	 1.f,  1.f,  0.f,  1.f,  1.f,
	 1.f, -1.f,  0.f,  1.f,  0.f
};

GLuint indices[] {
	0, 2, 1,
	0, 3, 2
}
```


__Vertex Shader__:

```cpp
#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 uvs;

out vec2 UVs;

void main() {
	gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
	UVs = uvs;
}
```


__Fragment Shader__:

```cpp
#version 330 core

out vec4 FragColor
in vec2 UVs;

uniform sampler2D screen;

void main() {
	FragColor = texture(screen, UVs);
}
```


__Compute Shader__:

```c++
#version 330 core

// How many invocations per workgroup
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D screen;

void main() {
	// stuff
}
```

Visto que não temos UVs (que teríamos no Frag. Shader) então ainda não temos forma de saber onde estamos no ecrã. Como? Compute Shaders não têm **user defined inputs** mas ainda assim têm **5** inputs.


##### Base Inputs
-> `gl_NumWorkGroups`:
	*uvec3* - tamanho dos workgroups neste compute shader

-> `gl_WorkGroupID`:
	*uvec3* - o ID do workgroup a que estamos atualmente

-> `gl_LocalInvocationID`:
	*uvec3* - o ID da invocação atual em respeito ao workgroup em que estamos

##### Shorthand Inputs
-> `gl_GlobalInvocationID`:
	*uvec3* - o ID da invocação atual em respeito ao Compute Shader
```glsl
  gl_GlobalInvocationID = gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocation
```

-> `gl_LocalInvocationIndex`:
	*uint*  - o index da invocação atual em respeito ao workgroup em que estamos:
```glsl
  gl_LocalInvocationIndex = 
	  gl_LocalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y +
	  gl_LocalInvocationID.y * gl_WorkGroupSize.x +
	  gl_LocalInvocationID.x
```

#### How to get UVs in a Compute Shader
```c++
#version 330 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D screen;

void main() {
	ivec2 pixel_coords = ivec2(gl_LocalInvocation);

	ivec2 dims = imageSize(screen);
	float x = -(float(pixel_coords.x * 2 - dims.x) / dims.x); // x -> [-1.f, 1.f]
	float y = -(float(pixel_coords.y * 2 - dims.y) / dims.y); // y -> [-1.f, 1.f]
}
```

