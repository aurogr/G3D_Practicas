#version 330 core

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexCoord;

uniform sampler2D colorTex;
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
vec3 Lpos_1 = vec3(0.0);
vec3 Ia = vec3(0.3);
vec3 Id_1 = vec3(1.0);
vec3 Is_1 = vec3(1.0);

// Per light definition
vec3 Lpos_2 = vec3(10.0, 5.0, -10.0);
vec3 Id_2 = vec3(1.0, 0.0, 0.0);
vec3 Is_2 = vec3(1.0, 0.0, 0.0);

out vec4 outColor;

void main() {
	N = normalize(vNormal);

	Ka = texture(colorTex, vTexCoord).rgb;
	Kd = texture(colorTex, vTexCoord).rgb;
	Ks = vec3(1.0);
	Ke = texture(emiTex, vTexCoord).rgb;

	outColor = vec4(shade(), 1.0);
}

vec3 shade(){
	vec3 shading = vec3(0.0);

	vec3 L_1 = normalize(Lpos_1 - vPos);
	vec3 L_2 = normalize(Lpos_2 - vPos);
	vec3 V = normalize(-vPos);

	// Ambient term
	vec3 ambient = Ia * Ka;
	shading += ambient;

	// Diffuse term (Lambert)
	vec3 diffuse_1 = Id_1 * Kd * clamp(dot(N, L_1), 0.0, 1.0);
	vec3 diffuse_2 = Id_2 * Kd * clamp(dot(N, L_2), 0.0, 1.0);
	shading += diffuse_1;
	shading += diffuse_2;

	// Specular term

	// Phong
	// vec3 R = normalize(reflect(-L, N)); // Phong
	// float specularFactor = clamp(dot(R, V), 0.0, 1.0);

	// Blinn-Phong
	vec3 H_1 = normalize(V + L_1);
	vec3 H_2 = normalize(V + L_2);
	float specularFactor_1 = clamp(dot(N, H_1), 0.0, 1.0);
	float specularFactor_2 = clamp(dot(N, H_2), 0.0, 1.0);

	vec3 specular_1 = Is_1 * Ks * pow(specularFactor_1, alpha);
	vec3 specular_2 = Is_2 * Ks * pow(specularFactor_2, alpha);
	shading += specular_1;
	shading += specular_2;

	// Emissive term
	shading += Ke;

	return shading;
}
