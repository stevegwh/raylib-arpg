//
// Created by Steve Wheeler on 03/08/2024.
//

#include "Explosion.hpp"
#include "components/sgTransform.hpp"
#include "components/Renderable.hpp"

namespace sage
{
	void Explosion::Restart()
	{
		scale = 0;
		auto& r = registry->get<Renderable>(entity);
		r.active = true;
	}
	
	void Explosion::Update()
	{
		if (scale >= maxScale) return;
		scale += increment * GetFrameTime();
		auto& transform = registry->get<sgTransform>(entity);
		transform.SetScale(scale, entity);
		if (scale >= maxScale)
		{
			auto& r = registry->get<Renderable>(entity);
			r.active = false;
		}
	}
	
	void Explosion::SetOrigin(Vector3 origin)
	{
		auto& transform = registry->get<sgTransform>(entity);
		transform.SetPosition(origin, entity);
	}

	Explosion::~Explosion()
	{
		UnloadShader(shader);
	}
	
	Explosion::Explosion(entt::registry* _registry)
	{
		registry = _registry;
		sphere = LoadModelFromMesh(GenMeshHemiSphere(1.0f, 16, 16));
		entity = registry->create();
		registry->emplace<sgTransform>(entity);
		auto& r = registry->emplace<Renderable>(entity, sphere, MatrixIdentity());
		r.hint = Color{255, 0, 0, 100};
		shader = ResourceManager::ShaderLoad(nullptr, "resources/shaders/glsl330/bloom.fs");
		r.model.materials[0].shader = shader;
	}
} // sage