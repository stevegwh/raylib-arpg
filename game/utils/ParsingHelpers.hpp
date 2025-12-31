//
// Created by steve on 07/01/2025.
//

#pragma once

#include "entt/entt.hpp"
#include <string>

namespace lq
{
    class Systems;
    namespace parsing
    {
        struct TextFunction
        {
            std::string name;
            std::string params;
        };

        std::string removeCommentsFromFile(const std::string& fileContents);
        std::string trim(const std::string& str);
        std::string trimWhiteSpaceFromFile(const std::string& fileContents);
        std::string normalizeLineEndings(const std::string& content);
        TextFunction getFunctionNameAndArgs(const std::string& input);
        std::function<bool()> GetConditionalStatement(
            const std::string& line, entt::registry* registry, Systems* sys);

    } // namespace parsing
} // namespace lq
