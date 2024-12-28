#version 330

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPosition;
out vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float seconds;

void main()
{
    vec2 uv = fragTexCoord;
    
    // Animate the distortion UV
    vec2 distortionUV = uv + vec2(seconds * 0.12, 0);
    
    // Sample the distortion texture with animated UV
    vec2 distortion = texture(texture1, distortionUV).rg * 2.0 - 1.0;
    
    // Apply distortion to original UV
    vec2 distortedUV = uv + distortion * 0.3;  // Adjust the 0.1 to control distortion strength
    
    float d = length(distortedUV);
    vec3 col = vec3(1.0, 2.0, 3.0);
    d = sin(d * 8.0 + seconds * 5.0) / 8.0;
    d = abs(d);
    d = 0.03 / d;
    
    // Incorporate distortion into the color
    col *= d;
    col += vec3(distortion, 0.0) * 0.3;  // Add some of the distortion to the color
    
    fragColor = vec4(col, 1.0);
}