//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include <vector>
#include <utility>
#include <unordered_map>
#include <string>

#include "raylib.h"

namespace sage
{
typedef std::vector<std::pair<std::string, std::vector<std::unordered_map<std::string, std::string>>>> SerializationData;
class Serializer
{
public:
    static Vector3 ConvertStringToVector3(const std::string& str);
    static void SerializeToFile(const SerializationData& serializeData);
    static SerializationData DeserializeFile(); // TODO: Currently does not register entities with register
};

} // sage
