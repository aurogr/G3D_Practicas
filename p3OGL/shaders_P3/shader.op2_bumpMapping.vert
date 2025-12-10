#version 330 core

in vec3 inPos;
in vec3 inNormal;
in vec3 inTangent;
in vec2 inTexCoord;

uniform mat4 modelViewProj;
uniform mat4 modelView; // los calculos de iluminacion se suelen hacer con los vectores en espacio de cámara
uniform mat4 normal;

out vec3 vPos;
out mat3 TBN;
out vec2 vTexCoord;

out vec4 outColor;

void main()
{
	vPos = (modelView * vec4(inPos, 1.0)).xyz;
	vec3 N = normalize((normal * vec4(inNormal, 0.0)).xyz);
	vec3 T = normalize((normal * vec4(inTangent, 0.0)).xyz);
	vec3 B = normalize((normal * vec4(cross(inNormal, inTangent), 0.0)).xyz);

	TBN = mat3(T, B, N);
	vTexCoord = inTexCoord;

	gl_Position =  modelViewProj * vec4(inPos, 1.0);
}


