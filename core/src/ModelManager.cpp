#include "ModelManager.hpp"

namespace sage
{

	void ModelManager::addModel(const std::string& path)
	{
		Model model = LoadModel(path.c_str());
		modelData.at(path) = model;
	}

	Model ModelManager::ModelLoad(const std::string& path)
	{
		if (!modelData.contains(path))
		{
			addModel(path);
		}

		return modelData.at(path);
	}
}