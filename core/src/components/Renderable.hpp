//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include <slib.hpp>

#include "raylib.h"
#include "raymath.h"

#include "raylib-cereal.hpp"
#include "cereal/cereal.hpp"

#include "cereal/types/string.hpp"

#include <string>
#include <optional>


namespace sage
{
	struct Renderable
	{
		Matrix initialTransform{};
		MaterialPaths materials;
		Model model{}; // was const
		std::optional<Shader> shader;
		std::string name = "Default";
		bool serializable = true;

		Renderable() = default;
		Renderable(const Renderable&) = delete;
		Renderable& operator=(const Renderable&) = delete;
		Renderable(Model _model, MaterialPaths _materials, Matrix _localTransform);
		Renderable(Model _model, Matrix _localTransform);
		~Renderable();

		template <class Archive>
		void save(Archive& archive) const
		{
			archive(
				model,
				name,
				materials,
				initialTransform
			);
		}

		template <class Archive>
		void load(Archive& archive)
		{
			archive(
				model,
				name,
				materials,
				initialTransform
			);

			char* _name = new char[this->name.size() + 1];
			model.meshes[0].name = _name;
			model.transform = initialTransform;

			// Set the textures of the model with their respective paths
			model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(materials.diffuse.c_str());
			model.materials[0].maps[MATERIAL_MAP_SPECULAR].texture = LoadTexture(materials.specular.c_str());
			model.materials[0].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture(materials.normal.c_str());

		}
	};
}
