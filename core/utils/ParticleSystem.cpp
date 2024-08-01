//
// Created by steve on 31/07/2024.
//

#include "ParticleSystem.hpp"


#include <iostream>

float GetRandomFloat(float from, float to) {
	float random = (float)GetRandomValue(0, RAND_MAX) / (float)RAND_MAX;
	return from + random * (to - from);
}
namespace sage
{

	ParticleSystem::~ParticleSystem()
	{
		UnloadShader(particleShader);
		rlUnloadShaderBuffer(ssbo0);
		rlUnloadShaderBuffer(ssbo1);
		rlUnloadShaderBuffer(ssbo2);
		rlUnloadVertexArray(particleVao);
	}

	ParticleSystem::ParticleSystem(Camera3D* _camera) : camera(_camera)
	{
		// Compute shader for updating particles.
		char* shaderCode = LoadFileText("resources/shaders/particle_compute.glsl");
		int shaderData = rlCompileShader(shaderCode, RL_COMPUTE_SHADER);
		computeShader = rlLoadComputeShaderProgram(shaderData);
		UnloadFileText(shaderCode);

		// Shader for constructing triangles and drawing.
		particleShader = LoadShader(
			"resources/shaders/particle_vertex.glsl",
			"resources/shaders/particle_fragment.glsl");

		//
		// Now we prepare the buffers that we connect to the shaders.
		// For each variable we want to give our particles, we create one buffer
		// called a Shader Storage Buffer Object containing a single variable type.
		//
		// We will use only Vector4 as particle variables, because data in buffers
		// requires very strict alignment rules.
		// You can send structs, but if not properly aligned will introduce many bugs.
		// For information on the std430 buffer layout see:
		// https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL).
		//

		auto* positions = (Vector4*)RL_MALLOC(sizeof(Vector4) * numParticles);
		auto* velocities = (Vector4*)RL_MALLOC(sizeof(Vector4) * numParticles);

		for (int i = 0; i < numParticles; i++)
		{
			// We only use the XYZ components of position and velocity.
			// Use the remainder for extra effects if needed, or create more buffers.
			positions[i] = (Vector4){
				GetRandomFloat(-0.5, 0.5),
				GetRandomFloat(-0.5, 0.5),
				GetRandomFloat(-0.5, 0.5),
				0,
			};
			velocities[i] = (Vector4){ 0, 0, 0, 0 };
		}

		// Load three buffers: Position, Velocity and Starting Position. Read/Write=RL_DYNAMIC_COPY.
		ssbo0 = rlLoadShaderBuffer(numParticles * sizeof(Vector4), positions, RL_DYNAMIC_COPY);
		ssbo1 = rlLoadShaderBuffer(numParticles * sizeof(Vector4), velocities, RL_DYNAMIC_COPY);
		ssbo2 = rlLoadShaderBuffer(numParticles * sizeof(Vector4), positions, RL_DYNAMIC_COPY);

		// For instancing we need a Vertex Array Object.
		// Raylib Mesh* is inefficient for millions of particles.
		// For info see: https://www.khronos.org/opengl/wiki/Vertex_Specification
		particleVao = rlLoadVertexArray();
		rlEnableVertexArray(particleVao);

		// Our base particle mesh is a triangle on the unit circle.
		// We will rotate and stretch the triangle in the vertex shader.
		Vector3 vertices[] = {
			{ -0.86, -0.5, 0.0 },
			{ 0.86, -0.5, 0.0 },
			{ 0.0f, 1.0f, 0.0f }
		};

		// Configure the vertex array with a single attribute of vec3.
		// This is the input to the vertex shader.
		rlEnableVertexAttribute(0);
		rlLoadVertexBuffer(vertices, sizeof(vertices), false); // dynamic=false
		rlSetVertexAttribute(0, 3, RL_FLOAT, false, 0, 0);
		rlDisableVertexArray(); // Stop editing.

		//Camera camera = { { 2, 2, 2 }, { 0, 0, 0 }, { 0, 1, 0 }, 35.0, CAMERA_PERSPECTIVE };


	}

	void ParticleSystem::Update()
	{
		float deltaTime = GetFrameTime();
		numInstances = (int)(instances_x1000 / 1000 * numParticles);

		// Compute Pass.
		rlEnableShader(computeShader);

		// Set our parameters. The indices are set in the shader.
		rlSetUniform(0, &time, SHADER_UNIFORM_FLOAT, 1);
		rlSetUniform(1, &timeScale, SHADER_UNIFORM_FLOAT, 1);
		rlSetUniform(2, &deltaTime, SHADER_UNIFORM_FLOAT, 1);
		rlSetUniform(3, &sigma, SHADER_UNIFORM_FLOAT, 1);
		rlSetUniform(4, &rho, SHADER_UNIFORM_FLOAT, 1);
		rlSetUniform(5, &beta, SHADER_UNIFORM_FLOAT, 1);

		rlBindShaderBuffer(ssbo0, 0);
		rlBindShaderBuffer(ssbo1, 1);
		rlBindShaderBuffer(ssbo2, 2);

		// We have numParticles/1024 workGroups. Each workgroup has size 1024.
		rlComputeShaderDispatch(numParticles / 1024, 1, 1);
		rlDisableShader();
		time += deltaTime;

//		Vector4* positions = (Vector4*)RL_MALLOC(sizeof(Vector4) * numParticles);
//		rlReadShaderBuffer(ssbo0, positions, sizeof(Vector4) * numParticles, 0);
//		for (int i = 0; i < 5; i++)  // Check first 5 particles
//		{
//			std::cout << TextFormat("Particle %d position: (%f, %f, %f)\n", i, positions[i].x, positions[i].y, positions[i].z) << std::endl;
//		}
//		RL_FREE(positions);
	}

	void ParticleSystem::Draw()
	{
		rlEnableShader(particleShader.id);

		// Because we use rlgl, we must take care of matrices ourselves.
		// We need to only pass the projection and view matrix.
		// These will be used to make the particle face the camera and such.
		Matrix projection = rlGetMatrixProjection();
		Matrix view = GetCameraMatrix(*camera);

		SetShaderValueMatrix(particleShader, 0, projection);
		SetShaderValueMatrix(particleShader, 1, view);
		SetShaderValue(particleShader, 2, &particleScale, SHADER_UNIFORM_FLOAT);

		rlBindShaderBuffer(ssbo0, 0);
		rlBindShaderBuffer(ssbo1, 1);

		// Draw the particles. Instancing will duplicate the vertices.
		rlEnableVertexArray(particleVao);
		rlDrawVertexArrayInstanced(0, 3, numInstances);
		rlDisableVertexArray();
		rlDisableShader();

		DrawCubeWires((Vector3){ 0, 0, 0 }, 1.0, 1.0, 1.0, DARKGRAY);
	}
} // sage