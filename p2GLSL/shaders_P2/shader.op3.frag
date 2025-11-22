#version 330 core

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// OPTIONAL PART 3: FOG /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexCoord;
uniform sampler2D colorTex;

vec3 N; // it's vNormal, we maintain it just so the formula looks pretty
float epsilon = 1e-3;

// Phong shading
vec3 shade();

// Per object properties (Material BRDF definition)
vec3 Ka;
vec3 Kd;
vec3 Ks;
float alpha = 500.0;

// Per light definition
vec3 Ia = vec3(0.3);
vec3 Id = vec3(1.0);
vec3 Is = vec3(1.0);
vec3 Lpos = vec3(0.0);

// Fog parameters
vec3 fogColor = vec3(0.0);
float fogDensity = 0.1;

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

	vec3 L = normalize(Lpos - vPos);
	// vPos is already in camera coordinates (camera is the origin) (0,0,0) - vPos = -vPos
	vec3 V = normalize(-vPos); 

	// Ambient term
	shading += Ia * Ka; // cos(theta) doesn't matter because ambient light goes everywhere

	// Diffuse term (Lambert)
	vec3 diffuse = Id * Kd * clamp(dot(N, L), 0.0, 1.0); // clamp assures that is always between 0 and 1
	shading += diffuse;

	//Specular term (Blinn-Phong)
	vec3 H = normalize(V + L);
	float specularFactor = clamp(dot(N, H), 0.0, 1.0);
	vec3 spec = Is * Ks * pow(specularFactor, alpha);
	shading += spec;

	// Fog
	float fogFactor = exp(-pow(fogDensity * (-vPos.z),2.0));
	shading = fogFactor * shading + (1-fogFactor) * fogColor;

	return shading;
}


