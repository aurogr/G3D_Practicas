#version 330 core

in vec3 vColor;
in vec3 vNormal;
in vec2 vTexCoord;

uniform sampler2D colorTex;

out vec4 outColor;

void main() {
	outColor = vec4(vNormal, 1.0);
}
