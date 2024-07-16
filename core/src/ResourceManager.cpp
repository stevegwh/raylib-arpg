//
// Created by Steve Wheeler on 16/07/2024.
//

#include "ResourceManager.hpp"
#include <unordered_map>

namespace sage
{

static std::unordered_map<std::string, Image> textureImages;

Image ResourceManager::LoadTexture(const std::string &path)
{
    if (textureImages.find(path) == textureImages.end())
    {
        Image img = LoadImage(path.c_str());
        textureImages[path] = img;
        return img;
    }
    return textureImages[path];
}

ResourceManager::~ResourceManager()
{
    for (auto kv : textureImages)
    {
        UnloadImage(kv.second);
    }
}
} // sage