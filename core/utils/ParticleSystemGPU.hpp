//
// Created by steve on 31/07/2024.
//

#pragma once

#include <raylib.h>

namespace sage
{

	class ParticleSystemGPU
	{
		Camera3D* camera;
		Shader particleShader;
		int computeShader;

		float time = 0;
		float timeScale = 0.2f;
		float sigma = 10;
		float rho = 28.0f;
		float beta = 8.0/3.0;
		float particleScale = 1.0;
		float instances_x1000 = 100.0;
		// Number of particles should be a multiple of 1024, our workgroup size (set in shader).
		int numParticles = 1024 * 100;
		int ssbo0;
		int ssbo1;
		int ssbo2;
		int particleVao;
		int numInstances;
	public:
		~ParticleSystemGPU();
		ParticleSystemGPU(Camera3D* _camera);
		void Update();
		void Draw();
	};

} // sage