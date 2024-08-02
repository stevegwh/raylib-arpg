#version 430

// Input vertex position of the base particle triangle
layout (location=0) in vec3 vertexPosition;

// Input uniform values
layout (location=0) uniform mat4 projectionMatrix;
layout (location=1) uniform mat4 viewMatrix;
layout (location=2) uniform float particleScale;

// Shader Storage Buffer Objects (SSBOs) for particle data
layout(std430, binding=0) buffer ssbo0 { vec4 positions[]; };
layout(std430, binding=1) buffer ssbo1 { vec4 velocities[]; };

// Output color to fragment shader
out vec4 fragColor;

void main()
{
    // Get the particle's velocity and position from SSBOs
    vec3 velocity = velocities[gl_InstanceID].xyz;
    vec3 position = positions[gl_InstanceID].xyz;

    // Set color based on velocity direction
    fragColor.rgb = abs(normalize(velocity)) + 0.2;
    fragColor.a = 1.0;

    // Scale the base triangle
    float scale = 0.005 * particleScale;
    vec3 vertexView = vertexPosition * scale;

    // Calculate velocity in view space
    vec2 velocityView = (viewMatrix * vec4(velocity, 0)).xy;
    float velocityAngle = atan(velocityView.y, velocityView.x);
    float speed = length(velocityView);

    // Rotate the triangle to point in the direction of movement
    float rot = velocityAngle - radians(90);
    vec2 xvec = vec2(cos(rot), sin(rot));
    vec2 yvec = vec2(-sin(rot), cos(rot));
    vertexView.xy = vertexView.x * xvec + vertexView.y * yvec;

    // Scale the tip of the triangle based on speed
    float isTip = float(gl_VertexID == 2);
    float arrowLength = speed * 0.05;
    vertexView.xy = vertexView.xy * (1 - isTip) + isTip * vertexView.xy * (arrowLength + 1);

    // Transform particle position to view space
    vec3 viewPos = (viewMatrix * vec4(position, 1.0)).xyz;

    // Add the rotated and scaled vertex offset to the view space position
    viewPos += vertexView;

    // Calculate final vertex position in clip space
    gl_Position = projectionMatrix * vec4(viewPos, 1.0);
}