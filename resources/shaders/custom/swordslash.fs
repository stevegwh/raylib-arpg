#version 330
in vec2 fragTexCoord;
in vec3 fragNormal;
out vec4 fragColor;
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float seconds;

// https://iquilezles.org/articles/palettes
vec3 palette(float t, vec3 a, vec3 b, vec3 c, vec3 d){ return a + b*cos( 6.28318*(c*t+d) ); }

const float ColorAnchor[3]  = float[]( 0.0, 0.5, 0.75 );
const vec4 ColorRamp[3]     = vec4[](
    vec4(0.0,0.0,0.0,0.0),
    vec4(0.933, 0.679, 0.378, 1.0),
    vec4(1.0, 0.992, 0.703, 1.0)
);

vec4 getColorRamp(float v, float feather){
    for(int i=0; i < 2; i++){
        if(v >= ColorAnchor[i] && v < ColorAnchor[i+1])
            return mix( ColorRamp[i], ColorRamp[i+1], smoothstep(ColorAnchor[i+1]-feather,ColorAnchor[i+1],v) );
    }
    return ColorRamp[2];
}

// Following the tutorial by SketchpunkLabs from: https://www.youtube.com/watch?v=uT5w3Bwhk4s
void main()
{
    vec2 uv = fragTexCoord;
    //uv.x += seconds * 2;
    float grad = mix(0.0, 2.0, fragTexCoord.x - 0.5);

    vec4 d = texture(texture1, fragTexCoord) * 0.1;
    vec4 n = texture(texture0, uv.xy + d.rg);
    n += grad;

    // "Realistic"
    vec4 colorMix = mix( vec4(1.0, 0.5, 0.0, 1.0), vec4(0.9, 0.9, 0.9, 1.0), fragTexCoord.x );

    // Most "realistic" (discards less)
    // Color offset (fragTexCoord.x - 0.45) creates negative values (black)
    //if (n.r < 0.5) discard; // Set to "< 0" or above
    //fragColor = colorMix * vec4(n.r, n.r, n.r, 1);
    // -----

    // Harder masks (Choose hard or soft)
    // Hard mask
    float nn = step(0.5, n.r);
    if (nn < 1.0) discard;
    // -----
    // Soft mask
    // float nn = smoothstep(0.5, 0.65, n.r);
    // if (nn < 0.3) discard; // Adjust this for feathering
    // -----
    fragColor = vec4(nn, nn, nn, 1) * colorMix;
    // -----

    // "Cartoony"
    // vec4 colramp = getColorRamp( clamp(n.r, 0.0, 1.0), 0.05 );
    // if (colramp.a < 0.5) discard;
    // fragColor = colramp;
    // -----
}