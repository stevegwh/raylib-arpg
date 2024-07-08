//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once
#include "raylib.h"
#include "raymath.h"
#include "cereal/cereal.hpp"

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
				CEREAL_NVP(modelPath),
				CEREAL_NVP(material.path),
				CEREAL_NVP(name),
				CEREAL_NVP(initialTransform.m0),
				CEREAL_NVP(initialTransform.m1),
				CEREAL_NVP(initialTransform.m2),
				CEREAL_NVP(initialTransform.m3),
				CEREAL_NVP(initialTransform.m4),
				CEREAL_NVP(initialTransform.m5),
				CEREAL_NVP(initialTransform.m6),
				CEREAL_NVP(initialTransform.m7),
				CEREAL_NVP(initialTransform.m8),
				CEREAL_NVP(initialTransform.m9),
				CEREAL_NVP(initialTransform.m10),
				CEREAL_NVP(initialTransform.m11),
				CEREAL_NVP(initialTransform.m12),
				CEREAL_NVP(initialTransform.m13),
				CEREAL_NVP(initialTransform.m14),
				CEREAL_NVP(initialTransform.m15));
		}

		template <class Archive>
		void load(Archive& archive)
		{
			archive(
				CEREAL_NVP(modelPath),
				CEREAL_NVP(material.path),
				CEREAL_NVP(name),
				CEREAL_NVP(initialTransform.m0),
				CEREAL_NVP(initialTransform.m1),
				CEREAL_NVP(initialTransform.m2),
				CEREAL_NVP(initialTransform.m3),
				CEREAL_NVP(initialTransform.m4),
				CEREAL_NVP(initialTransform.m5),
				CEREAL_NVP(initialTransform.m6),
				CEREAL_NVP(initialTransform.m7),
				CEREAL_NVP(initialTransform.m8),
				CEREAL_NVP(initialTransform.m9),
				CEREAL_NVP(initialTransform.m10),
				CEREAL_NVP(initialTransform.m11),
				CEREAL_NVP(initialTransform.m12),
				CEREAL_NVP(initialTransform.m13),
				CEREAL_NVP(initialTransform.m14),
				CEREAL_NVP(initialTransform.m15));

			model = LoadModel(modelPath.c_str());
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
