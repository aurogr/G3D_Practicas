#version 330 core

out vec4 outColor;
uniform sampler2D colorTex;
uniform sampler2D auxiliarTex;
in vec2 vTexCoord;

void main()
{
	vec4 colorDiscard = texture(auxiliarTex, vTexCoord);
	outColor = texture(colorTex, vTexCoord);   

	float brightness = dot(colorDiscard.rgb, vec3(0.333));

	if(brightness <= 0.5)
		discard;

}
