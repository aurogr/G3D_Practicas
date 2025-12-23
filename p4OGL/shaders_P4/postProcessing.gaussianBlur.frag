#version 330 core

/// GAUSSIAN BLUR ///
#define MASK_SIZE 25u
const vec2 texIdx[MASK_SIZE] = vec2[](
vec2(-2.0,2.0), vec2(-1.0,2.0), vec2(0.0,2.0), vec2(1.0,2.0), vec2(2.0,2.0),
vec2(-2.0,1.0), vec2(-1.0,1.0), vec2(0.0,1.0), vec2(1.0,1.0), vec2(2.0,1.0),
vec2(-2.0,0.0), vec2(-1.0,0.0), vec2(0.0,0.0), vec2(1.0,0.0), vec2(2.0,0.0),
vec2(-2.0,-1.0), vec2(-1.0,-1.0), vec2(0.0,-1.0), vec2(1.0,-1.0), vec2(2.0,-1.0),
vec2(-2.0,-2.0), vec2(-1.0,-2.0), vec2(0.0,-2.0), vec2(1.0,-2.0), vec2(2.0,-2.0));

const float maskFactor = float (1.0/65.0);

const float mask[MASK_SIZE] = float[](
1.0*maskFactor, 2.0*maskFactor, 3.0*maskFactor,2.0*maskFactor, 1.0*maskFactor,
2.0*maskFactor, 3.0*maskFactor, 4.0*maskFactor,3.0*maskFactor, 2.0*maskFactor,
3.0*maskFactor, 4.0*maskFactor, 5.0*maskFactor,4.0*maskFactor, 3.0*maskFactor,
2.0*maskFactor, 3.0*maskFactor, 4.0*maskFactor,3.0*maskFactor, 2.0*maskFactor,
1.0*maskFactor, 2.0*maskFactor, 3.0*maskFactor,2.0*maskFactor, 1.0*maskFactor);

out vec4 outColor; 
in vec2 texCoord; 
uniform sampler2D colorTex; 
uniform bool horizontal;

void main()
{
	//Sería más rápido utilizar una variable uniform el tamaño de la textura.
	vec2 ts = vec2(1.0) / vec2 (textureSize (colorTex,0));
	vec4 color = vec4 (0.0);

	if (horizontal) {
		for (uint i = 0u; i < MASK_SIZE; i++)
		{
			vec2 iidx = texCoord + vec2(ts.x * texIdx[i].x, 0.0);
			color += texture(colorTex, iidx) * mask[i];
		}
		//color = vec4(1.0, 0.0, 0.0, 1.0); // Debug: show red for horizontal pass
	} else {
		for (uint i = 0u; i < MASK_SIZE; i++)
		{
			vec2 iidx = texCoord + vec2(0.0, ts.y * texIdx[i].y);
			color += texture(colorTex, iidx) * mask[i];
		}
		//color = vec4(0.0, 0.0, 1.0, 1.0); // Debug: show blue for vertical pass
	}

	outColor = vec4(color.rgb, 1.0);
}