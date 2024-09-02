#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform float seconds;
uniform vec2 screenSize;

vec3 palette(float t) {
    // Fire palette
    vec3 a = vec3(0.5, 0.0, 0.0);   // dark red
    vec3 b = vec3(0.5, 0.2, 0.0);   // orange-red
    vec3 c = vec3(1.0, 1.0, 1.0);   // keeps the oscillation
    vec3 d = vec3(0.0, 0.1, 0.2);   // slight blue shift for variety

    return a + b*cos(6.28318*(c*t + d));
}

void main() {
    vec2 uv = fragTexCoord * 2.0 - 1.0;
    uv.x *= screenSize.x / screenSize.y;
    vec2 uv0 = uv;
    vec3 tmpColor = vec3(0.0);
    
    for (float i = 0.0; i < 4.0; i++) {
        uv = fract(uv*1.5) - 0.5;
        float d = length(uv)*exp(-length(uv0));
        vec3 col = palette(length(uv0) + i*0.4 + seconds*0.4);
        d = sin(d*8.0 + seconds)/8.0;
        d = abs(d);
        d = pow(0.01/d, 1.2);
        tmpColor += col*d;
    }
    
    // Calculate distance from center
    vec2 center = vec2(0.5, 0.5);
    float dist = length(fragTexCoord - center);
   
    // Set the radius of the circle (0.5 for a circle that touches the edges of the quad)
    float radius = 0.5; // Adjust this value to change the size of the circular clip
   
    // Clip everything outside the circle
    if (dist > radius) {
        discard;
    }
    
    // Adjust brightness threshold for more fire-like effect
    float brightness = dot(tmpColor, vec3(0.299, 0.587, 0.114));
    if (brightness < 0.1) { // Lowered threshold for more dark areas
        discard;
    }
    
    // Enhance reds and oranges
    tmpColor = pow(tmpColor, vec3(0.8, 1.2, 1.5));
    
    finalColor = vec4(tmpColor, 1.0);
}