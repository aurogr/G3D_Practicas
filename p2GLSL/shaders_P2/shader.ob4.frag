// Disney 2012 PBR shader with only diffuse and specular terms
// No sheen, no clearcoat and no subsurface scattering (blend)

#version 330 core

const float PI = 3.141592653589793;
const float EPSILON = 1e-5;
const int MATERIAL_COUNT = 3;

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexCoord;

uniform sampler2D emiTex;

// Per object properties (Material BRDF definition)
struct DisneyMaterial {
	vec3 baseColor;			// Albedo
	float subsurface; 		// Subsurface scattering blend
	float metallic; 		// 0 = Dielectric, 1 = Metal
	float specular; 		// Specular amount (dielectric specular scale)
	float specularTint; 	// Tint specular towards base color
	float roughness; 		// Surface roughness
	float sheen; 			// Velvet-like sheen
	float sheenTint; 		// Tint for sheen
	float clearcoat;  		// Clearcoat layer
	float clearcoatGloss; 	// Clearcoat glossiness
};

// Per light definition
vec3 Lpos = vec3(0.0, 0.0, 5.0);
vec3 Ia = vec3(0.3);
vec3 lightColor = vec3(1.0);
float lightIntensity = 1.0;

out vec4 outColor;

// D: GTR2 (GGX / Trowbridge-Reitz) normal distribution function
float D_GGX(float NdotH, float alpha) {
	float a2 = alpha * alpha;
	float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
	denom = PI * denom * denom;
	return a2 / max(denom, EPSILON);
}

// F: Schlick Fresnel approximation
vec3 F_Schlick(vec3 F0, float cosTheta) {
    // Schlick: F = F0 + (1 - F0) * (1 - cosTheta)^5
	float pow5 = pow(1.0 - cosTheta, 5.0);
	return F0 + (1.0 - F0) * pow5;
}

// Schlick-GGX geometry helper for one direction
float G_Schlick_GGX(float NdotV, float k) {
	return NdotV / (NdotV * (1.0 - k) + k);
}

// G: Smith geometry term using Schlick-GGX
float G_Smith(float NdotV, float NdotL, float k) {
	return G_Schlick_GGX(NdotV, k) * G_Schlick_GGX(NdotL, k);
}

// Schlick scalar for sheen
float F_SchlickScalar(float F0, float cosTheta) {
	float pow5 = pow(1.0 - cosTheta, 5.0);
	return F0 * pow5;
}

// GTR1 used by Disney for clearcoat (normalized)
float D_GTR1(float NdotH, float alpha) {
	return 0.0;
}

// Main Disney BRDF evaluation
vec3 disneyBRDF(DisneyMaterial m, vec3 N, vec3 V, vec3 L) {
	vec3 brdfColor = vec3(0.0);

	vec3 H = normalize(L + V);
	float NdotL = clamp(dot(N, L), 0.0, 1.0); // cos(theta_l)
	float NdotV = clamp(dot(N, V), 0.0, 1.0); // cos(theta_v)
	float NdotH = clamp(dot(N, H), 0.0, 1.0); // cos(theta_h)
	float VdotH = clamp(dot(V, H), 0.0, 1.0); // cos(theta_d)

    // Specular term (GGX)
	// TO DO

	// Map roughness to alpha
	// alpha es la inversa de la fuerza de la componente especular, es roughness^2
	float alpha = max(m.roughness * m.roughness, EPSILON); // se usa max para que no se vaya a 0

	// Compute D term (Microfacets normal distribution)
	// a partir del angulo h (h es la normal de la microfaceta) y un parametro alpha digo cuantas normales están alineadas con la normal de la superficie (¿esta superficie es más rugosa o pulida?)
	float D = D_GGX(NdotH, alpha);

	// Use specularTint (solo en materiales dielectricos)
	vec3 tint = normalize(m.baseColor); // normalmente se divide entre la luminancia del color pero por simplificar y porque el efecto es parecido normalizamos
	vec3 specularColor = mix(vec3(1.0), tint, m.specularTint); // interpolacion lineal entre blanco y el color según specularTint
	vec3 dielectricF0 = vec3(0.04) * m.specular * specularColor; // valor de fresnel por defecto de media en los materiales (por defecto 0.04) 
									// modela el reflejo especular en ángulos rasantes

	// Compute F term (Fresnel)
	vec3 F0 = mix(dielectricF0, m.baseColor, m.metallic); // nos quedamos con el valor dielectrico o con el color base, según cuanto de metálico es el material
	vec3 F = F_Schlick(F0, VdotH);

	// Compute G term (Visibility)
	// shadowmasking
	// en una superficie compuesta por microfacetas pueden ocurrir microoclusiones
	// compensar con la energía que se nos genera con D y F porque no tenemos considerar que la especular de una superficie con microfacetas es tan potente como la de una lisa
	float k = m.roughness + 1.0; // tenemos que tener en cuenta la roughness (en una superficie muy pulida no hay mucha oclusión)
	k = (k * k) / 8; // cosas de mates que no nos importan! lo importante es saber que depende del roughness

	float G = G_Smith(NdotV, NdotL, k);

	// Valor final de la componente especular
	vec3 specular = (D * F * G) / max(4.0 * NdotL * NdotV, EPSILON);
	brdfColor += specular;

    // Diffuse term
	// Lambert: fd = baseColor / PI
	// conjunto de evaluaciones del material que son iguales en todas las direcciones
	// la división entre PI es para controlar la energía que genera

	// la fórmula de Lambert se comporta de manera errática con ángulos rasantes, para ello se introducen dos términos más fdL y fdV
	// dependen de FD90
	float F_D90 = 0.5 + 2 * m.roughness * pow(VdotH, 2.0);

	float fdL = 1.0 + (F_D90 - 1.0) * pow(1.0 - NdotL, 5.0);
	float fdV = 1.0 + (F_D90 - 1.0) * pow(1.0 - NdotV, 5.0);
	
	// Valor final de la componente diffusa
	vec3 diffuse = (m.baseColor / PI) * fdL * fdV;

	brdfColor += diffuse * (1.0 - m.metallic); // Metals do not have a diffuse (Lambertian) component

    // Sheen term
	// se caracteriza por mayor brillo en ángulos rasantes, que puede ser del color del material
	float F_Sheen = F_SchlickScalar(m.sheen, NdotH);
	vec3 sheen = F_Sheen * mix(vec3(1.0), tint, m.sheenTint);
	brdfColor += sheen;
	
    // Clearcoat term
	vec3 clearcoat = vec3(0.0);
	if(m.clearcoat > 0.0) {
		brdfColor += clearcoat;
	}

    // Subsurface term (approx — slight retro-reflection mix)
	if(m.subsurface > 0.0) {
		float LdotH = clamp(dot(L, H), 0.0, 1.0);
		// diffuse = mix(diffuse, ?, clamp(m.subsurface, 0.0, 1.0));
	}

	return brdfColor * NdotL;
}

// PBR Disney 2012 shading
vec3 shade(DisneyMaterial m);

void main() {
	// Hard-coded palette: Array of DisneyMaterial
	int selectedMaterial = 2;
	const DisneyMaterial materials[MATERIAL_COUNT] = DisneyMaterial[](
			// 0: Brushed Gold (metallic)
		DisneyMaterial(vec3(1.00, 0.84, 0.40), 0.0, 1.0, 0.0, 0.5, 0.28, 0.0, 0.0, 0.0, 0.0),
			// 1: Soft Plastic Toy (dielectric)
		DisneyMaterial(vec3(0.95, 0.3, 0.05), 0.0, 0.0, 0.64, 0.75, 0.15, 0.0, 0.0, 0.0, 0.0),
			// 2: Fabric velvet (dielectric)
		DisneyMaterial(vec3(0.420, 0.020, 0.094), 0.2, 0.0, 0.0, 0.0, 1.0, 0.9, 0.0, 0.0, 0.0)
	);

	outColor = vec4(shade(materials[selectedMaterial]), 1.0);
}

vec3 shade(DisneyMaterial m) {
	vec3 shading = vec3(0.0);

	vec3 N = normalize(vNormal);
	vec3 V = normalize(-vPos);
	vec3 L = normalize(Lpos - vPos);
	
	// Ambient term
	// TO DO
	vec3 ambient = Ia * m.baseColor * (1.0 - m.metallic);
	shading += ambient;
	
	// Specular term + Diffuse term
	// Evaluate BRDF (per-light). For multiple lights sum each light contribution
	// TO DO
	// si tuvieramos varias fuentes habria que hacer un bucle y multiplicar las propiedades del material por cada una de las luces y por el ángulo de la luz
	// en este caso no es necesario el angulo porque ya viene en la BRDF
	vec3 brdf = disneyBRDF(m, N, V, L); // The BRDF is already multiplied inside by NdotL (cosine factor)
	shading += lightIntensity * lightColor * brdf; // Multiply by light properties

	// Emissive term
	// TO DO
	//shading += texture(emiTex, vTexCoord).rgb;

    // Gamma-correct to sRGB
	// TO DO
	// pasar de una curva recta a una curva exponencial
	// se percibe el color de manera no lineal
	// tiene que ver con como funcionan los monitores y como percibe el color el ojo humano
	// pasamos el color final a un espacio de color diferente
	shading = pow(clamp(shading, 0.0, 1.0), vec3(1.0/2.2));

	return shading;
}
