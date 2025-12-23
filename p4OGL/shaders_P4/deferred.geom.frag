#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gEmissive;

in vec3 vPos;
in vec3 vColor;
in vec3 vNormal;
in vec2 vTexCoord;

uniform sampler2D colorTex;
uniform sampler2D specularTex;
uniform sampler2D emiTex;

void main()
{    
    // store the information in the gbuffer
    gPosition = vPos;
    gNormal = normalize(vNormal);
    gAlbedoSpec.rgb = texture(colorTex, vTexCoord).rgb;
    gAlbedoSpec.a = texture(specularTex, vTexCoord).r; // specular intensity in gAlbedoSpec's alpha component
    gEmissive = texture(emiTex, vTexCoord).rgb;
}  