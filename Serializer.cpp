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

    // Create a stringstream to tokenize the input string
    std::stringstream ss(str);
    std::string token;

    // Tokenize the string by commas and extract floating-point numbers
    std::vector<float> values;
    while (std::getline(ss, token, ',')) {
        values.push_back(std::stof(token));
    }

    // Extract the values into the Vector3 struct
    if (values.size() == 3) {
        vec.x = values[0];
        vec.y = values[1];
        vec.z = values[2];
    } else {
        // Handle incorrect input format
        std::cerr << "Error: Invalid input format. Expected format: 'x,y,z'" << std::endl;
        // Set default values or throw an exception as needed
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

        // Create or access the JSON object for the entity ID
        if (!j.contains(entityId)) {
            j[entityId] = json::object();
        }

        for (const auto& componentNameEntry : componentMap)
        {
            const std::string& componentName = componentNameEntry.first;
            const auto& fieldMap = componentNameEntry.second;

            // Create or access the JSON object for the component name
            if (!j[entityId].contains(componentName)) {
                j[entityId][componentName] = json::object();
            }

            for (const auto& fieldEntry : fieldMap)
            {
                const std::string& fieldName = fieldEntry.first;
                const std::string& fieldValue = fieldEntry.second;

                // Set the field value in the component object
                j[entityId][componentName][fieldName] = fieldValue;
            }
        }
    }

    std::ofstream o("pretty.json");
    o << std::setw(4) << j << std::endl;
}



// std::vector<std::pair<std::string, std::vector<std::unordered_map<std::string, std::string>>>> SerializationData;
// { "TypeName": [ { "FieldName": "10004", "FieldName": "10.00, 0.00, 20.00" } ] }
// { "Transform": [ { "EntityId": "10004", "Position": "10.00, 0.00, 20.00" } ] }
SerializationData Serializer::DeserializeFile()
{
    std::ifstream i("pretty.json");
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

    return deserializeData;
}

} // sage