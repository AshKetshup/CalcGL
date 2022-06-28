#version 330 core

out vec4 color;

uniform sampler2D u_texture;
uniform vec2	  iResolution;

void main() {
	vec2 uv = gl_PointCoord / iResolution - vec2(.5);

	color = texture(u_texture, uv);
}