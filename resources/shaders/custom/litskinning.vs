#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
in vec4 vertexBoneIds;
in vec4 vertexBoneWeights;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

#define MAX_BONE_NUM 128
uniform mat4 boneMatrices[MAX_BONE_NUM];

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// NOTE: Add here your custom variables

void main()
{
    // Skeletal animation
    int boneIndex0 = int(vertexBoneIds.x);
    int boneIndex1 = int(vertexBoneIds.y);
    int boneIndex2 = int(vertexBoneIds.z);
    int boneIndex3 = int(vertexBoneIds.w);

    vec4 skinnedPosition =
    vertexBoneWeights.x * (boneMatrices[boneIndex0] * vec4(vertexPosition, 1.0f)) +
    vertexBoneWeights.y * (boneMatrices[boneIndex1] * vec4(vertexPosition, 1.0f)) +
    vertexBoneWeights.z * (boneMatrices[boneIndex2] * vec4(vertexPosition, 1.0f)) +
    vertexBoneWeights.w * (boneMatrices[boneIndex3] * vec4(vertexPosition, 1.0f));

    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel * skinnedPosition);
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    // Apply skinning to normals as well
    vec3 skinnedNormal =
    vertexBoneWeights.x * (mat3(boneMatrices[boneIndex0]) * vertexNormal) +
    vertexBoneWeights.y * (mat3(boneMatrices[boneIndex1]) * vertexNormal) +
    vertexBoneWeights.z * (mat3(boneMatrices[boneIndex2]) * vertexNormal) +
    vertexBoneWeights.w * (mat3(boneMatrices[boneIndex3]) * vertexNormal);

    fragNormal = normalize(vec3(matNormal * vec4(skinnedNormal, 0.0)));

    // Calculate final vertex position
    gl_Position = mvp * skinnedPosition;
}