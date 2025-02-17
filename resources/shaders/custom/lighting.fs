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
#include "lighting.fs"

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
	
	finalColor = Lighting_CalculateLighting(texelColor);
}