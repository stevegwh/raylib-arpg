//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include <string>

#include "cereal/cereal.hpp"


struct EditorSettings
{
	std::string resourcePath = "resources/";

	template <class Archive>
	void save(Archive& archive) const
	{
		archive(CEREAL_NVP(resourcePath));
	}

	template <class Archive>
	void load(Archive& archive)
	{
		archive(CEREAL_NVP(resourcePath));
	}
};
