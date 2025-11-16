#version 330 core

in vec3 inPos;
in vec3 inColor;
in vec2 inTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 vColor;
out vec2 texCoord;

void main()
{
	vColor = inColor;
	texCoord = inTexCoord;
	gl_Position =  proj * view * model * vec4(inPos, 1.0);
}
