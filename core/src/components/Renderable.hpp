//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once
#include "raylib.h"
#include "raymath.h"

#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"

#include "../Material.hpp"

#include <string>
#include <optional>

namespace sage
{
	struct Renderable
	{
		Matrix initialTransform{};
		Material material;
		std::string modelPath;
		Model model{}; // was const
		std::optional<Shader> shader;
		std::string name = "Default";
		bool serializable = true;

		Renderable() = default;
		Renderable(const Renderable&) = delete;
		Renderable& operator=(const Renderable&) = delete;
		Renderable(Model _model, Material _material, std::string _modelPath, Matrix _localTransform);
		Renderable(Model _model, std::string _modelPath, Matrix _localTransform);
		~Renderable();

		template <class Archive>
		void save(Archive& archive) const
		{
			archive(
				modelPath,
				model,
				material.path,
				name,
				initialTransform
			);
		}

		template <class Archive>
		void load(Archive& archive)
		{
			archive(
				modelPath,
				model,
				material.path,
				name,
				initialTransform
			);

			
			char* _name = new char[this->name.size() + 1];
			model.meshes[0].name = _name;
			if (!material.path.empty())
			{
				material.diffuse = LoadTexture(material.path.c_str());
				model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = material.diffuse;
			}
		}
	};
}
