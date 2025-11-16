#version 330 core

in vec3 inPos;
in vec3 inColor;
in vec3 inNormal;

uniform mat4 modelViewProj;
uniform mat4 modelView; // los calculos de iluminacion se suelen hacer con los vectores en espacio de cámara
uniform mat4 normal;

vec3 vPos;
vec3 N;
float epsilon = 1e-3;

out vec3 vColor;

// Gouraud shading
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

void main()
{
	vPos = (modelView * vec4(inPos, 1.0)).xyz;
	N = normalize((normal * vec4(inNormal, 0.0)).xyz);

	Ka = inColor;
	Kd = inColor;
	Ks = vec3(1.0);

	vColor = shade();
	gl_Position =  modelViewProj * vec4(inPos, 1.0);
}

vec3 shade(){
	vec3 shading = vec3(0.0);

	vec3 L = normalize(Lpos - vPos);
	// vPos is already in camera coordinates (camera is the origin) (0,0,0) - vPos = -vPos
	vec3 V = normalize(-vPos); 

	// Ambient term
	shading += Ia * Ka; // cos(theta) doesn't matter because ambient light goes everywhere

	// Diffuse term (Lambert)
	shading += Id * Kd * clamp(dot(N, L), 0.0, 1.0); // clamp assures that is always between 0 and 1

	//Specular term
	vec3 R = normalize(reflect(-L, N));
	float specularFactor = clamp(dot(R, V), 0.0, 1.0);
	shading += Is * Ks * pow(specularFactor, alpha);

	return shading;
}


