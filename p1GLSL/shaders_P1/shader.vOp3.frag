#version 330 core

out vec4 outColor;
uniform sampler2D auxiliarTex;
in vec2 vTexCoord;

float hash( in ivec2 p )
{                         
    // 2D -> 1D
    int n = p.x*3 + p.y*113;

    // 1D hash by Hugo Elias
	n = (n << 13) ^ n;
    n = n * (n * n * 15731 + 789221) + 1376312589;
    return -1.0+2.0*float( n & 0x0fffffff)/float(0x0fffffff);
}

float noise( in vec2 uv) 
{
	 ivec2 i = ivec2(floor( uv ));
	 vec2 f = fract( uv );

	// quintic interpolant
    vec2 u = f*f*f*(f*(f*6.0-15.0)+10.0);

	return mix( mix( hash( i + ivec2(0,0) ), 
                     hash( i + ivec2(1,0) ), u.x),
                mix( hash( i + ivec2(0,1) ), 
                     hash( i + ivec2(1,1) ), u.x), u.y);
}

void main()
{
	float scale = 1;
	vec2 uv = vTexCoord * scale;
	float value;

	// fbm (4 octaves)
	uv *= 8.0;
	mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 );
	value = 0.5000*noise( uv ); uv = m*uv;
	value += 0.2500*noise( uv ); uv = m*uv;
	value += 0.1250*noise( uv ); uv = m*uv;
	value += 0.0625*noise( uv ); uv = m*uv;

	value = 0.5 + 0.5*value;

	vec3 color1 = vec3(0.1,0.9,0);
	vec3 color2 = vec3(0,0,0.6);
	outColor= vec4(smoothstep(color1,color2,vec3(value)), 1.0);
	
	// fragment discard
	vec4 colorDiscard = texture(auxiliarTex, vTexCoord);
	float brightness = dot(colorDiscard.rgb, vec3(0.333));
	if(brightness <= 0.5)
		discard;
}

