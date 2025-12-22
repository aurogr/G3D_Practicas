#version 330 core

out vec2 texCoord; 
in vec3 inPos;	
layout(location = 1) out vec4 outVertex; 

void main()
{
	texCoord = inPos.xy*0.5+vec2(0.5);
	gl_Position = vec4 (inPos,1.0); 
	outVertex = vec4(inPos,1.0); 
}
