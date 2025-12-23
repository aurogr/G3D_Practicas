#version 330 core

/// DEPTH OF FIELD ///

out vec4 outColor; 
in vec2 texCoord; 
uniform sampler2D colorTex; 
uniform sampler2D depthTex;

const float focalDistance = -25.0;
const float maxDistanceFactor = 1.0/5.0;
uniform float near = 1.0; 		// same as glm::persperctive
uniform float far = 50.0;		

uniform float convMask[25];
uniform int convMaskSize;
uniform vec2 convTexId[25];

void main()
{
	//Seria mas rapido utilizar una variable uniform el tamaño de la textura.
	vec2 ts = vec2(1.0) / vec2 (textureSize (colorTex,0));

	//depthTex is in NDC. Transform to camera space.
	float depthNDC = texture(depthTex,texCoord).x;
	float depthCS = (near * far) / (far - depthNDC * (far - near));

	float dof = abs(depthCS - abs(focalDistance)) * maxDistanceFactor;
	dof = clamp (dof, 0.0, 1.0);
	dof *= dof; 

	vec4 color = vec4 (0.0);
	for (int i = 0; i < convMaskSize; i++)
	{
		vec2 iidx = texCoord + ts * convTexId[i] * dof;
		color += texture(colorTex, iidx,0.0) * convMask[i];
	}

	outColor = color;
}