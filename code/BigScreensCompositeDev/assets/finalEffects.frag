#version 150 core
#extension all : warn

in vec2 oTexCoord;

uniform sampler2D fboTexture;

uniform vec2 texSize;
uniform float time;
uniform float volume;

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
	
	vec4 oricol = texture( fboTexture, texCoord );
    vec4 col;
	
	// start with the source texture and misalign the rays it a bit

	if ( oricol.r != oricol.g && oricol.r != oricol.b ) {
		col.r = texture( fboTexture, vec2(texCoord.x + (0.0005 * volume),
                                          texCoord.y ) ).x;
		col.g = texture( fboTexture, vec2(texCoord.x + (0.000 * volume),
                                          texCoord.y ) ).y;
		col.b = texture( fboTexture, vec2(texCoord.x + (-0.0005 * volume),
                                          texCoord.y ) ).z;
		col.a = texture( fboTexture, vec2(texCoord.x,
                                          texCoord.y ) ).a;
	}
	else {
		col = oricol;
	}
    
	for( i = -2 ; i < 2; i++ )
	{
        for (j = -2; j < 2; j++ )
        {
            sum += texture( fboTexture, texCoord + vec2(j, i)*0.0005) * 0.5;
        }
	}
	if ( col.r < 0.3 )
    {
		col = sum*sum*0.012 + col;
    }
    else
    {
        if ( col.r < 0.5 )
        {
            col = sum*sum*0.009 + col;
        }
        else
        {
            col = sum*sum*0.0075 + col;
        }
    }
	
	// subtle zoom in/out
	vec2 uv = 0.5 + ( texCoord - 0.5 ) * ( 0.98 + 0.006 * sin( 0.9 * time ) );
	
	// contrast curve
    col = clamp( col * 0.5 + 0.5 * col * col * 1.2, 0.0, 1.0 );
	
	// vignette
    col *= 0.8 + 0.2 * 16.0 * uv.x * uv.y * ( 1.0 - uv.x ) * ( 1.0 - uv.y );
	
	//color tint
//    col *= vec4( 0.9, 1.0, 0.7, 1.0 );
	
	//scanline (last 2 constants are crawl speed and size)
    //TODO make size dependent on viewport
    col *= 0.9 + 0.1 * sin( 10.0 * time + uv.y * 1500.0 );
	
	//flickering (semi-randomized)
//    col *= 1.0 - 0.07 * rand( vec2( time, tan( time ) ) );
	
	color = col;
}




