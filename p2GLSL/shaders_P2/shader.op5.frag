#version 330 core

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// PART 3.2: SPOTLIGHT /////////////////////////////
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

// Spotlight definition
vec3 Ia = vec3(0.3);
vec3 Id = vec3(1.0);
vec3 Is = vec3(1.0);
vec3 Lpos = vec3(0.0);
vec3 D = normalize(vec3(0.0, 0.0, -0.9));	// direction (normalized)
float thetaU = radians(15);					// cutoff angle of aperture
float thetaP = radians(8.0);				// gloom angle
float m = 5.0;								// aperture attenuation factor

// Attenuation variables 
float d0 = 10.0;  // mult value for light intensity

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

	// Distance attenuation
	float d = length(Lpos - vPos);
	float fdist = (d0 / (d*d + epsilon));

	// Spotlight attenuation
	float spDist = pow(clamp((dot(-L,D) - cos(thetaU)) / (cos(thetaP)- cos(thetaU)),0.0,1.0),m);

	// Ambient term
	shading += Ia * Ka; // cos(theta) doesn't matter because ambient light goes everywhere

	// Diffuse term (Lambert)
	vec3 diffuse = Id * Kd * clamp(dot(N, L), 0.0, 1.0); // clamp assures that is always between 0 and 1

	//Specular term (Blinn-Phong)
	vec3 H = normalize(V + L);
	float specularFactor = clamp(dot(N, H), 0.0, 1.0);
	vec3 spec = Is * Ks * pow(specularFactor, alpha);

	shading += spDist * (diffuse + spec);

	return shading;
}


