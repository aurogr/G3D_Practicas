#version 330 core

out vec4 outColor; 
in vec2 texCoord; 
uniform sampler2D colorTex; 
uniform bool horizontal;

uniform float convMask[25];
uniform int convMaskSize;
uniform vec2 convTexId[25];

void main()
{
	//Sería más rápido utilizar una variable uniform el tamaño de la textura.
	vec2 ts = vec2(1.0) / vec2 (textureSize (colorTex,0));
	vec4 color = vec4 (0.0);

	if (horizontal) {
		for (int i = 0; i < convMaskSize; i++)
		{
			vec2 iidx = texCoord + vec2(ts.x * convTexId[i].x, 0.0);
			color += texture(colorTex, iidx) * convMask[i];
		}
		//color = vec4(1.0, 0.0, 0.0, 1.0); // Debug: show red for horizontal pass
	} else {
		for (int i = 0; i < convMaskSize; i++)
		{
			vec2 iidx = texCoord + vec2(0.0, ts.y * convTexId[i].y);
			color += texture(colorTex, iidx) * convMask[i];
		}
		//color = vec4(0.0, 0.0, 1.0, 1.0); // Debug: show blue for vertical pass
	}

	outColor = vec4(color.rgb, 1.0);
}