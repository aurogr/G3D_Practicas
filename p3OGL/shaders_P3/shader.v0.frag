#version 330 core

in vec3 vPos;
in vec3 vColor;
in vec3 vNormal;

uniform sampler2D colorTex;
uniform sampler2D emiTex;

out vec4 outColor;

vec3 N;
float epsilon = 1e-3;

// Per object properties (Material BRDF definition)
vec3 Ka;
vec3 Kd;
vec3 Ks;
vec3 Ke;
float alpha = 1700.0;

// Per light definition
vec3 Lpos = vec3(0.0);
vec3 Ia = vec3(0.3);
vec3 Id = vec3(1.0);
vec3 Is = vec3(1.0);

// Phong shading
vec3 shade();

void main() {
	N = normalize(vNormal);

	Ka = vColor;
	Kd = vColor;
	Ks = vec3(1.0);
	Ke = vec3(0.0);

	outColor = vec4(shade(), 1.0);
}

vec3 shade() {
	vec3 shading = vec3(0.0);
	vec3 L = normalize(Lpos - vPos);

	// Ambient term
	vec3 ambient = Ia * Ka;
	shading += ambient;

	// Diffuse term (Lambert)
	vec3 diffuse = Id * Kd * dot(L, N);
	shading += clamp(diffuse, 0.0, 1.0);

	// Specular term
	vec3 V = normalize(-vPos);

	// vec3 R = normalize(reflect(-L, N)); // Phong
	// float factor = max(dot(R, V), epsilon); 

	vec3 H = normalize(L + V); // Blinn-Phong
	float factor = max(dot(N, H), epsilon);

	vec3 specular = Is * Ks * pow(factor, alpha);
	shading += clamp(specular, 0.0, 1.0);

	// Emissive term
	shading += Ke;

	return shading;
}
