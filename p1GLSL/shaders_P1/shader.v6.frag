#version 330 core

in vec3 vColor;
in vec2 texCoord;
in vec3 vNormal;

out vec4 outColor;

uniform sampler2D colorTex;

void main()
{
	outColor = vec4(abs(vNormal), 1.0);
	//texture(colorTex, texCoord);
}
