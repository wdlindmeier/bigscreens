#version 150 core
#extension all : warn

in vec2 oTexCoord;

uniform sampler2D fboTexture;

uniform vec2 texSize;
uniform float time;

out vec4 color;

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
	vec4 sum = vec4(0);
	vec2 texCoord = vec2( oTexCoord );
	int j;
	int i;
	
	for( i = -4 ; i < 4; i++ )
	{
        for (j = -4; j < 4; j++ )
        {
            sum += texture( fboTexture, texCoord + vec2(j, i)*0.003) * 0.25;
        }
	}
	if ( texture( fboTexture, texCoord).r < 0.3 )
    {
		color = sum*sum*0.012 + texture( fboTexture, texCoord);
    }
    else
    {
        if ( texture( fboTexture, texCoord).r < 0.5 )
        {
            color = sum*sum*0.019 + texture( fboTexture, texCoord );
        }
        else
        {
            color = sum*sum*0.0075 + texture( fboTexture, texCoord );
        }
    }
	
	// subtle zoom in/out
	vec2 uv = 0.5 + ( texCoord - 0.5 ) * ( 0.98 + 0.006 * sin( 0.9 * time ) );
	
	vec3 oricol = texture( fboTexture, texCoord ).xyz;
    vec3 col;
	
	// start with the source texture and misalign the rays it a bit
    // TODO animate misalignment upon hit or similar event
	col.r = texture( fboTexture, vec2( texCoord.x + 0.003, texCoord.y ) ).x;
    col.g = texture( fboTexture, vec2( texCoord.x + 0.000, texCoord.y ) ).y;
    col.b = texture( fboTexture, vec2( texCoord.x - 0.003, texCoord.y ) ).z;
	
//	color = vec4( col, 1.0 );
	
	// contrast curve
    col = clamp( col * 0.5 + 0.5 * col * col * 1.2, 0.0, 1.0 );
	
	//vignette
    col *= 0.6 + 0.4 * 16.0 * uv.x * uv.y * ( 1.0 - uv.x ) * ( 1.0 - uv.y );
	
	//color tint
    col *= vec3( 0.9, 1.0, 0.7 );
	
	//scanline (last 2 constants are crawl speed and size)
    //TODO make size dependent on viewport
    col *= 0.8 + 0.2 * sin( 10.0 * time + uv.y * 900.0 );
	
	//flickering (semi-randomized)
    col *= 1.0 - 0.07 * rand( vec2( time, tan( time ) ) );
	
	//smoothen
//    float comp = smoothstep( 0.2, 0.7, sin(time) );
//    col = mix( col, oricol, clamp( -2.0 + 2.0 * texCoord.x + 3.0 * comp, 0.0, 1.0 ) );
	
	color = (vec4(col,1.0) * color);
//	color = vec4(1.0);
}




