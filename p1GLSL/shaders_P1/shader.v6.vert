#version 330 core

in vec3 inPos;
in vec3 inColor;
in vec2 inTexCoord;
in vec3 inNormal;

uniform mat4 modelViewProj;
uniform mat4 normal;

out vec3 vColor;
out vec2 texCoord;
out vec3 vNormal;

void main()
{
	vColor = inColor;
	vNormal = (normal * vec4(inNormal, 1.0)).xyz;
	texCoord = inTexCoord;
	gl_Position =  modelViewProj * vec4(inPos, 1.0);
}
