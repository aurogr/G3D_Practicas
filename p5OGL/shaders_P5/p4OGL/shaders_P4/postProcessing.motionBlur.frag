#version 330 core

/// FOR MOTION BLUR (code in main) ///

out vec4 outColor; 
in vec2 texCoord; 
uniform sampler2D colorTex; 

void main()
{
	//outColor = vec4(texCoord,vec2(1.0)); 
	outColor = vec4(textureLod(colorTex, texCoord,0).xyz, 0.6); 
}