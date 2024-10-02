#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

#define     MAX_LIGHTS              25
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
    float attenuation;// Not used, currently
};

// Input lighting values
uniform int lightsCount;
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;
uniform float gamma;

vec4 GetLightingValue()
{
    // Light control parameters (to be replaced with uniforms later)
    const float lightReachMultiplier = 0.8;// Adjusts how far the light reaches
    const float lightPowerMultiplier = 1.4;// Adjusts the intensity of the light

    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    for (int i = 0; i < lightsCount; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);
            float attenuation = 1.0;

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].target - lights[i].position);
            }

            if (lights[i].type == LIGHT_POINT)
            {
                vec3 lightVector = lights[i].position - fragPosition;
                light = normalize(lightVector);

                float distance = length(lightVector);
                float constant = 1.0;
                float linear = 0.09 / lightReachMultiplier;
                float quadratic = 0.032 / (lightReachMultiplier * lightReachMultiplier);
                attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

                // Apply power multiplier
                attenuation = pow(attenuation, 1.0 / lightPowerMultiplier);
            }

            float NdotL = max(dot(normal, light), 0.0);
            lightDot += lights[i].color.rgb * NdotL * attenuation * lightPowerMultiplier;

            float specCo = 0.0;
            if (NdotL > 0.0) specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0);// 16 refers to shine
            specCo *= 0.5;
            specular += specCo * attenuation * lightPowerMultiplier;
        }
    }

    vec4 result = (texelColor*((colDiffuse + vec4(specular, 1.0))*vec4(lightDot, 1.0)));
    result += texelColor*(ambient/10.0)*colDiffuse;

    // Gamma correction
    return pow(result, vec4(1.0/gamma));
}

void main()
{
    finalColor = GetLightingValue();
}