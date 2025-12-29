#version 330 core

in vec2 texCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gEmissive;

vec3 fragPos;
vec3 normal;
vec3 albedo;
float specular;
vec3 emissive;
float epsilon = 1e-3;

// Per object properties (Material BRDF definition)
float alpha = 500.0;

// Per light definition
vec3 Ia = vec3(0.1);
vec3 Id = vec3(1.0);
vec3 Is = vec3(0.7);
vec3 Lpos = vec3(0.0);

out vec4 outColor;

// Phong shading
vec3 shade();

void main() {
	fragPos = texture(gPosition, texCoord).rgb;
    normal = texture(gNormal, texCoord).rgb;
    albedo = texture(gAlbedoSpec, texCoord).rgb;
    specular = texture(gAlbedoSpec, texCoord).a;
	emissive = texture(gEmissive, texCoord).rgb;

	outColor = vec4(shade(), 1.0);
}


vec3 shade() {
	vec3 shading = vec3(0.0);
	vec3 L = normalize(Lpos - fragPos);

	// Ambient term
	vec3 ambient = Ia * albedo;
	shading += ambient;

	// Diffuse term (Lambert)
	vec3 diffuse = Id * albedo * dot(normal, L);
	shading += clamp(diffuse, 0.0, 1.0);

	// Specular term
	vec3 V = normalize(-fragPos);

	vec3 H = normalize(L + V); // Blinn-Phong
	float factor = max(dot(normal, H), epsilon);

	vec3 specular = Is * specular * pow(factor, alpha);
	shading += clamp(specular, 0.0, 1.0);

	shading += emissive;
	// Emissive term
	return shading;
}