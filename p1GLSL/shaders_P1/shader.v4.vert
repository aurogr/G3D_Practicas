#version 330 core

in vec3 inPos;
in vec3 inColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 vColor;

void main()
{
	vColor = inColor;
	gl_Position =  proj * view * model * vec4(inPos, 1.0);
}
