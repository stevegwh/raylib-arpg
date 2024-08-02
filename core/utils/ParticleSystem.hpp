//
// Created by steve on 31/07/2024.
//

#pragma once

#include <raylib.h>

namespace sage
{

	class ParticleSystem
	{

		Camera3D* camera;
		Vector3 origin;
		Shader particleShader;
		int computeShader;

		float time = 0;
		float timeScale = 0.2f;
		float sigma = 10;
		float rho = 28.0f;
		float beta = 8.0 / 3.0;
		float particleScale = 1.0;
		float instances_x1000 = 100.0;
		// Number of particles should be a multiple of 1024, our workgroup size (set in shader).
		int numParticles = 1024 * 100;
		int ssbo0;
		int ssbo1;
		int ssbo2;
		int particleVao;
		int numInstances;
		void resetParticles();
	public:
		bool enabled = false;
		~ParticleSystem();
		ParticleSystem(Camera3D* _camera);
		void Enable(bool enable = true);
		void Update();
		void Draw();
		void SetOrigin(Vector3 _origin);
	};

} // sage