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


//static void ProcessMaterialsOBJ(Material *materials, tinyobj_material_t *mats, int materialCount)
//{
//    // Init model mats
//    for (int m = 0; m < materialCount; m++)
//    {
//        // Init material to default
//        // NOTE: Uses default shader, which only supports MATERIAL_MAP_DIFFUSE
//        materials[m] = LoadMaterialDefault();
//
//        // Get default texture, in case no texture is defined
//        // NOTE: rlgl default texture is a 1x1 pixel UNCOMPRESSED_R8G8B8A8
//        materials[m].maps[MATERIAL_MAP_DIFFUSE].texture = (Texture2D){ rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
//
//        if (mats[m].diffuse_texname != NULL) materials[m].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(mats[m].diffuse_texname);  //char *diffuse_texname; // map_Kd
//        else materials[m].maps[MATERIAL_MAP_DIFFUSE].color = (Color){ (unsigned char)(mats[m].diffuse[0]*255.0f), (unsigned char)(mats[m].diffuse[1]*255.0f), (unsigned char)(mats[m].diffuse[2] * 255.0f), 255 }; //float diffuse[3];
//        materials[m].maps[MATERIAL_MAP_DIFFUSE].value = 0.0f;
//
//        if (mats[m].specular_texname != NULL) materials[m].maps[MATERIAL_MAP_SPECULAR].texture = LoadTexture(mats[m].specular_texname);  //char *specular_texname; // map_Ks
//        materials[m].maps[MATERIAL_MAP_SPECULAR].color = (Color){ (unsigned char)(mats[m].specular[0]*255.0f), (unsigned char)(mats[m].specular[1]*255.0f), (unsigned char)(mats[m].specular[2] * 255.0f), 255 }; //float specular[3];
//        materials[m].maps[MATERIAL_MAP_SPECULAR].value = 0.0f;
//
//        if (mats[m].bump_texname != NULL) materials[m].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture(mats[m].bump_texname);  //char *bump_texname; // map_bump, bump
//        materials[m].maps[MATERIAL_MAP_NORMAL].color = WHITE;
//        materials[m].maps[MATERIAL_MAP_NORMAL].value = mats[m].shininess;
//
//        materials[m].maps[MATERIAL_MAP_EMISSION].color = (Color){ (unsigned char)(mats[m].emission[0]*255.0f), (unsigned char)(mats[m].emission[1]*255.0f), (unsigned char)(mats[m].emission[2] * 255.0f), 255 }; //float emission[3];
//
//        if (mats[m].displacement_texname != NULL) materials[m].maps[MATERIAL_MAP_HEIGHT].texture = LoadTexture(mats[m].displacement_texname);  //char *displacement_texname; // disp
//    }
//}

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

			model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(materials.diffuse.c_str());
			// ProcessMaterialsOBJ here

		}
	};
}
