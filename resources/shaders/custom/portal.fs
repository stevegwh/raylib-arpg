#version 330
in vec2 fragTexCoord;
in vec3 fragNormal;
out vec4 fragColor;
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float seconds;

// https://iquilezles.org/articles/palettes
vec3 palette(float t, vec3 a, vec3 b, vec3 c, vec3 d){ return a + b*cos( 6.28318*(c*t+d) ); }

const float ColorAnchor[3] = float[]( 0.0, 0.5, 0.75 );
const vec4 ColorRamp[3] = vec4[](
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

// Portal looking thing
// Following the tutorial by SketchpunkLabs from: https://www.youtube.com/watch?v=uT5w3Bwhk4s
void main()
{
    vec2 res = vec2(1280,720);
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragTexCoord;
    uv.y += seconds * 0.2;
    vec4 d = texture(texture1, fragTexCoord) * 0.5;
    vec4 n = texture(texture0, uv + d.rg);
    
    // Normal color gradient
    fragColor = vec4(n.r, n.r, n.r, 1);
    fragColor *= mix( vec4(0.5, 0.0, 1.0, 1.0), vec4(0.75, 0.0, 0.25, 1.0), fragTexCoord.y );
    fragColor.a = 0.5;

    // Circular mask
    vec2 center = vec2(0.5, 0.5);
    float radius = 0.4;
    float dist = distance(fragTexCoord, center);
    float circle = smoothstep(radius, radius - 0.01, dist);

    // Apply the circular mask
    fragColor.a *= circle;
    if (fragColor.a < 0.2) discard;
}