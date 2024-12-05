#version 330

// Raylib defaults start
// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
in vec4 vertextangent;
in vec2 vertextexcoord2;
in vec4 vertexBoneIds;
in vec4 vertexBoneWeights;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matView;
uniform mat4 matProjection;
uniform mat4 matModel;
uniform mat4 matNormal;

#define MAX_BONE_NUM 128
uniform mat4 boneMatrices[MAX_BONE_NUM];

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
// Raylib defaults end

// Custom
uniform int skinned;

void main()
{
    // MacOS GLSL is strange and doesn't work well when you pass an int as a bool.
    int TRUE = 1;
    int FALSE = 0;
    
	vec4 pos = vec4(vertexPosition, 1.0);
	vec3 normal = vertexNormal;
	
	if (skinned == 1)
	{
	    int boneIndex0 = int(vertexBoneIds.x);
		int boneIndex1 = int(vertexBoneIds.y);
		int boneIndex2 = int(vertexBoneIds.z);
		int boneIndex3 = int(vertexBoneIds.w);
		
		pos =
			vertexBoneWeights.x * (boneMatrices[boneIndex0] * vec4(vertexPosition, 1.0f)) +
			vertexBoneWeights.y * (boneMatrices[boneIndex1] * vec4(vertexPosition, 1.0f)) + 
			vertexBoneWeights.z * (boneMatrices[boneIndex2] * vec4(vertexPosition, 1.0f)) + 
			vertexBoneWeights.w * (boneMatrices[boneIndex3] * vec4(vertexPosition, 1.0f));
			
		normal =
			vertexBoneWeights.x * (mat3(boneMatrices[boneIndex0]) * vertexNormal) +
			vertexBoneWeights.y * (mat3(boneMatrices[boneIndex1]) * vertexNormal) +
			vertexBoneWeights.z * (mat3(boneMatrices[boneIndex2]) * vertexNormal) +
			vertexBoneWeights.w * (mat3(boneMatrices[boneIndex3]) * vertexNormal);
	
	}
	
	fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));

    // Calculate final vertex position
    gl_Position = mvp * pos;
}