#version 330 core

in vec3 vPos;
in mat3 TBN;
in vec2 vTexCoord;

uniform sampler2D colorTex;
uniform sampler2D specularTex;
uniform sampler2D normalTex;
uniform sampler2D emiTex;

vec3 N;
float epsilon = 1e-3;

// Phong shading
vec3 shade();

// Per object properties (Material BRDF definition)
vec3 Ka;
vec3 Kd;
vec3 Ks;
vec3 Ke;
float alpha = 500.0;

// Per light definition
vec3 Lpos = vec3(0.0);
vec3 Ia = vec3(0.3);
vec3 Id = vec3(1.0);
vec3 Is = vec3(1.0);

out vec4 outColor;

void main() {
	N = TBN * (normalize(texture(normalTex, vTexCoord).rgb) * 2.0 -1.0);
	Ka = texture(colorTex, vTexCoord).rgb;
	Kd = texture(colorTex, vTexCoord).rgb;
	Ks = texture(specularTex, vTexCoord).rgb;;
	Ke = texture(emiTex, vTexCoord).rgb;

	outColor = vec4(shade(), 1.0);
}

vec3 shade(){
	vec3 shading = vec3(0.0);

	vec3 L = normalize(Lpos - vPos);
	vec3 V = normalize(-vPos);

	// Ambient term
	vec3 ambient = Ia * Ka;
	shading += ambient;

	// Diffuse term (Lambert)
	vec3 diffuse = Id * Kd * clamp(dot(N, L), 0.0, 1.0);
	shading += diffuse;

	// Specular term

	// Phong
	// vec3 R = normalize(reflect(-L, N)); // Phong
	// float specularFactor = clamp(dot(R, V), 0.0, 1.0);

	// Blinn-Phong
	vec3 H = normalize(V + L);
	float specularFactor = clamp(dot(N, H), 0.0, 1.0);

	vec3 specular = Is * Ks * pow(specularFactor, alpha);
	shading += specular;

	// Emissive term
	shading += Ke;

	return shading;
}
