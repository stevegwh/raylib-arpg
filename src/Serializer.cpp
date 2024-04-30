//
// Created by Steve Wheeler on 21/03/2024.
//

#include "Serializer.hpp"

#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

namespace sage
{

Vector3 Serializer::ConvertStringToVector3(const std::string& str) 
{
    Vector3 vec;
    std::stringstream ss(str);
    std::string token;

    std::vector<float> values;
    while (std::getline(ss, token, ',')) {
        values.push_back(std::stof(token));
    }

    if (values.size() == 3)
    {
        vec.x = values[0];
        vec.y = values[1];
        vec.z = values[2];
    }
    else
    {
        std::cerr << "Error: Invalid input format. Expected format: 'x,y,z'" << std::endl;
    }

    return vec;
}

void Serializer::SerializeToFile(const SerializationData& serializeData)
{
    json j;

    for (const auto& entityEntry : serializeData)
    {
        const std::string& entityId = entityEntry.first;
        const auto& componentMap = entityEntry.second;

        if (!j.contains(entityId))
        {
            j[entityId] = json::object();
        }

        for (const auto& componentNameEntry : componentMap)
        {
            const std::string& componentName = componentNameEntry.first;
            const auto& fieldMap = componentNameEntry.second;

            // Create or access the JSON object for the component name
            if (!j[entityId].contains(componentName))
            {
                j[entityId][componentName] = json::object();
            }

            for (const auto& fieldEntry : fieldMap)
            {
                const std::string& fieldName = fieldEntry.first;
                const std::string& fieldValue = fieldEntry.second;

                j[entityId][componentName][fieldName] = fieldValue;
            }
        }
    }

    std::ofstream o("pretty.json");
    o << std::setw(4) << j << std::endl;
}

// { "EntityId" { "TypeName": { "FieldName": "Value" } } }
// { "EntityId": { "Transform" { "Position": "10.00, 0.00, 20.00" } ] }
std::optional<SerializationData> Serializer::DeserializeFile()
{
    std::optional<SerializationData> data;
    std::ifstream i("pretty.json");

    if (!i.is_open() || i.fail()) return data;
    json j = json::parse(i);
    i.close();

    SerializationData deserializeData;

    // Iterate over each entity in the JSON object
    for (const auto& entityEntry : j.items())
    {
        const std::string& entityId = entityEntry.key();
        const auto& componentObjects = entityEntry.value();

        // Map to store components for the current entity
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> componentMap;

        // Iterate over each component in the entity
        for (const auto& componentNameEntry : componentObjects.items())
        {
            const std::string& componentName = componentNameEntry.key();
            const auto& fieldMap = componentNameEntry.value();

            // Map to store fields for the current component
            std::unordered_map<std::string, std::string> fieldMapForComponent;

            // Iterate over each field in the component
            for (const auto& fieldEntry : fieldMap.items())
            {
                const std::string& fieldName = fieldEntry.key();
                const std::string& fieldValue = fieldEntry.value();

                // Add the field to the field map for the component
                fieldMapForComponent[fieldName] = fieldValue;
            }

            // Add the field map for the current component to the component map for the entity
            componentMap[componentName] = fieldMapForComponent;
        }

        // Add the component map for the current entity to the deserialized data
        deserializeData[entityId] = componentMap;
    }
    data = deserializeData;
    return data;
}

} // sage