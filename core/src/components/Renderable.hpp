//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once
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
		std::string material;
		std::string modelPath;
		Model model{}; // was const
		std::optional<Shader> shader;
		std::string name = "Default";
		bool serializable = true;

		Renderable() = default;
		Renderable(const Renderable&) = delete;
		Renderable& operator=(const Renderable&) = delete;
		Renderable(Model _model, std::string _material, std::string _modelPath, Matrix _localTransform);
		Renderable(Model _model, std::string _modelPath, Matrix _localTransform);
		~Renderable();

		template <class Archive>
		void save(Archive& archive) const
		{
			archive(
				model,
				name,
				material,
				initialTransform
			);
		}

		template <class Archive>
		void load(Archive& archive)
		{
			archive(
				model,
				material,
				name,
				initialTransform
			);

			char* _name = new char[this->name.size() + 1];
			model.meshes[0].name = _name;
			model.transform = initialTransform;
			model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture("resources/models/obj/PolyAdventureTexture_01.png");
		}
	};
}
