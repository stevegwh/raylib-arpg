#pragma once

#include "raylib.h"

#include <unordered_map>
#include <string>


namespace sage
{
	class ModelManager
	{
		static void addModel(const std::string& path);
		static std::unordered_map<std::string, Model> modelData; // Path -> Entity
	public:
		static Model ModelLoad(const std::string& path);
	};
}