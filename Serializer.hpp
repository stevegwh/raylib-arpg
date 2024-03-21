//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include <vector>
#include <utility>
#include <unordered_map>
#include <string>

namespace sage
{

class Serializer
{
public:
    static void SerializeToFile(const std::vector<std::pair<std::string, std::vector<std::unordered_map<std::string, std::string>>>>& serializeData) ;

};

} // sage
