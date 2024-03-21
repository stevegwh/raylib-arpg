//
// Created by Steve Wheeler on 21/03/2024.
//

#include "Serializer.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sage
{
// { {(string)"ComponentName", { (string)"FieldName" : (unordered_map<string,string>){{fieldname: value}, etc...} } }
void Serializer::SerializeToFile(const std::vector<std::pair<std::string, std::vector<std::unordered_map<std::string, std::string>>>>& serializeData)
{
    json j;

    for (const auto& type : serializeData)
    {
        std::string typeName = type.first;

        for (const auto& components : type.second)
        {
            json componentJson;

            for (const auto& field : components)
            {
                componentJson[field.first] = field.second;
            }

            // Add the serialized component to the array under the type name
            j[typeName].push_back(componentJson);
        }
    }

    std::ofstream o("pretty.json");
    o << std::setw(4) << j << std::endl;
}

} // sage