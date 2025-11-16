#version 330 core

// Uniform matrix variables
uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform mat4 normal;
uniform mat4 modelView;
uniform mat4 modelViewProj;

// Uniform texture variables
uniform sampler2D colorTex;
uniform sampler2D emiTex;
uniform sampler2D normalTex;
uniform sampler2D specularTex;
uniform sampler2D auxiliarTex;

out vec4 outColor;

void main() {
	outColor = vec4(1.0);
}
