#version 330 core

out vec4 outColor;


void main()
{
	vec3 color = (gl_PrimitiveID % 2 == 0) ? vec3 (1.0) : vec3(0.0, 0.0, 1.0); 
	outColor = vec4(color, 1.0); 
}
