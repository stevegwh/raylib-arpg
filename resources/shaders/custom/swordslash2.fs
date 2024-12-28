#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Glow parameters
uniform float glowIntensity = 5.0;
uniform vec3 glowColor = vec3(1.0, 1.0, 1.0);

// Output fragment color
out vec4 finalColor;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    // Calculate the luminance of the texel
    float luminance = dot(texelColor.rgb, vec3(0.299, 0.587, 0.114));

    // Create glow effect based on luminance
    vec3 glowEffect = glowColor * luminance * glowIntensity;

    // Combine original color with glow
    vec3 finalRGB = texelColor.rgb + glowEffect;

    // Apply diffuse color and output
    finalColor = vec4(finalRGB, texelColor.a) * colDiffuse;
}