#version 330 core

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexCoord;
uniform sampler2D colorTex;

vec3 N; // it's vNormal, we maintain it just so the formula looks pretty
float epsilon = 1e-3;

// Gouraud shading
vec3 shade();

// Per object properties (Material BRDF definition)
vec3 Ka;
vec3 Kd;
vec3 Ks;
float alpha = 500.0;

// Per light definition
uniform vec3 inLightPos;
uniform vec3 inLightIa;
uniform vec3 inLightId;
uniform vec3 inLightIs;

out vec4 outColor;

void main()
{
	N = normalize(vNormal); // we have to normalice again after passing from vertex shader to fragment shader

	Ka = texture(colorTex,vTexCoord).xyz;
	Kd = texture(colorTex,vTexCoord).xyz;
	Ks = vec3(1.0);

	outColor = vec4(shade(), 1.0);
}

vec3 shade(){
	vec3 shading = vec3(0.0);

	vec3 L = normalize(inLightPos - vPos);
	// vPos is already in camera coordinates (camera is the origin) (0,0,0) - vPos = -vPos
	vec3 V = normalize(-vPos); 

	// Ambient term
	shading += inLightIa * Ka; // cos(theta) doesn't matter because ambient light goes everywhere

	// Diffuse term (Lambert)
	shading += inLightId * Kd * clamp(dot(N, L), 0.0, 1.0); // clamp assures that is always between 0 and 1

	//Specular term

	// Phong
	// vec3 R = normalize(reflect(-L, N)); 
	// float specularFactor = clamp(dot(R, V), 0.0, 1.0);

	// Blinn-Phong (better because it doesn't need to calculate R)
	vec3 H = normalize(V + L);
	float specularFactor = clamp(dot(N, H), 0.0, 1.0);

	shading += inLightIs * Ks * pow(specularFactor, alpha);

	return shading;
}


