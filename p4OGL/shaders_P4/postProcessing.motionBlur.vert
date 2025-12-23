#version 330 core

out vec2 texCoord; 
in vec3 inPos;	

void main()
{
	texCoord = inPos.xy*0.5+vec2(0.5);
	gl_Position = vec4 (inPos,1.0); 
}
