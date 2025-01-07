//
// Created by steve on 07/01/2025.
//

#pragma once

#include <string>

namespace sage::parsing
{
    struct TextFunction
    {
        std::string name;
        std::string params;
    };

    std::string trimAll(const std::string& fileContents);
    std::string trim(const std::string& str);
    std::string normalizeLineEndings(const std::string& content);
    TextFunction getFunctionNameAndArgs(const std::string& input);

} // namespace sage::parsing
