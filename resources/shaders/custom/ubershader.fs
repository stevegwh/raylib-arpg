#version 330

// Raylib defaults start
// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform vec4 colDiffuse;
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
// Raylib defaults end

// Output fragment color
out vec4 finalColor;

// Custom
uniform uint lit;
uniform uint hasEmissionTex;
uniform uint hasEmissionCol;
uniform sampler2D emissionMap;
uniform vec4 colEmission;

#include "lighting.fs"

void main()
{
    // MacOS GLSL is strange and doesn't work well when you pass an int as a bool.
    uint TRUE = uint(1);
    uint FALSE = uint(0);

    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    if (lit == TRUE)
    {
       finalColor = Lighting_CalculateLighting(texelColor);
    }
    else
    {
       finalColor = texelColor * colDiffuse * fragColor;
    }

    if (hasEmissionTex == TRUE)
    {
        vec4 emissionTexelCol = texture(emissionMap, fragTexCoord);
        finalColor = finalColor + emissionTexelCol;
    }
    else if (hasEmissionCol == TRUE)
    {
        finalColor = finalColor + colEmission;
    }
}
