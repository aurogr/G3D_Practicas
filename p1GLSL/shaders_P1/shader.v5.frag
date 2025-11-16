#version 330 core

in vec3 vColor;
in vec2 texCoord;

out vec4 outColor;

uniform sampler2D colorTex;

void main()
{
	outColor = texture(colorTex, texCoord);
}
