//
// Created by Steve Wheeler on 21/03/2024.
//

#include "Serializer.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sage
{
// { {"ComponentName", { "FieldName" : value, etc... } }
void Serializer::SerializeToFile(const std::vector<std::pair<std::string, std::unordered_map<std::string, std::string>>>& serializeData)
{
    json j;

    for (const auto& component: serializeData)
    {
        std::string type = component.first;

        for (const auto& field: component.second)
        {
            j[type][field.first] = field.second;
        }
    }

}
} // sage