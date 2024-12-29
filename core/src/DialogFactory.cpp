//
// Created by Steve Wheeler on 29/11/2024.
//

#include "DialogFactory.hpp"

#include "components/DialogComponent.hpp"
#include "components/QuestComponents.hpp"
#include "GameData.hpp"
#include "NpcManager.hpp"
#include "QuestManager.hpp"

#include "components/sgTransform.hpp"
#include "raylib.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;

namespace sage
{

    static std::string trim(const std::string& str)
    {
        auto start = str.find_first_not_of(" \t\n\r");
        auto end = str.find_last_not_of(" \t\n\r");
        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    }

    static std::string normalizeLineEndings(const std::string& content)
    {
        std::string normalized = content;
        // First replace all CRLF with LF
        size_t pos = normalized.find("\r\n");
        while (pos != std::string::npos)
        {
            normalized.replace(pos, 2, "\n");
            pos = normalized.find("\r\n", pos);
        }
        // Then replace any remaining CR with LF
        pos = normalized.find('\r');
        while (pos != std::string::npos)
        {
            normalized.replace(pos, 1, "\n");
            pos = normalized.find('\r', pos);
        }
        return normalized;
    }

    static std::unordered_map<std::string, std::string> extractVariables(const std::string& content)
    {
        std::unordered_map<std::string, std::string> variables;
        std::istringstream stream(content);
        std::string line;
        bool inVariableBlock = false;

        while (std::getline(stream, line, '\n'))
        {
            line = trim(line);

            if (line == "#variables start")
            {
                inVariableBlock = true;
                continue;
            }

            if (line == "#variables end")
            {
                inVariableBlock = false;
                continue;
            }

            if (inVariableBlock)
            {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos)
                {
                    std::string key = trim(line.substr(0, colonPos));
                    std::string value = trim(line.substr(colonPos + 1));
                    variables[key] = value;
                }
            }
        }

        return variables;
    }

    static std::string substituteVariables(
        const std::string& content, const std::unordered_map<std::string, std::string>& variables)
    {
        std::string result = content;

        for (const auto& [varName, value] : variables)
        {

            result = std::regex_replace(result, std::regex(R"(\$)" + varName), value);
        }

        return result;
    }

    static std::string preprocessDialog(const std::string& content)
    {
        auto variables = extractVariables(content);
        return substituteVariables(content, variables);
    }

    std::pair<std::string, std::string> getFunctionNameAndArgs(const std::string& input)
    {
        std::string trimmedInput = trim(input);

        // Adjusted regex pattern to allow broader parameters
        std::regex pattern(R"(^(\w+)\(([^)]+)\)$)");
        std::smatch match;

        if (std::regex_match(trimmedInput, match, pattern))
        {
            std::string functionName = match[1]; // First capture group
            std::string parameter = match[2];    // Second capture group

            return {functionName, parameter};
        }
        else
        {

            return {trimmedInput, ""};
        }
    }

    void DialogFactory::parseNode(
        dialog::Conversation* conversation,
        const std::string& nodeName,
        const std::string& content,
        const std::vector<std::vector<std::string>>& optionData)
    {
        auto node = std::make_unique<dialog::ConversationNode>(conversation);
        node->title = nodeName;
        node->content = content;

        std::optional<std::function<bool()>> condition;

        for (const auto& option : optionData)
        {
            if (option.at(0) == "if")
            {
                assert(!condition.has_value()); // if blocks must be closed with end. No nesting allowed (yet).
                // TODO: Should check if condition is related to quests
                bool negateCondition = false;
                unsigned int parameterIndex = 1;
                if (option.at(1) == "not")
                {
                    negateCondition = true;
                    parameterIndex = 2;
                }
                // TODO "AND" and "OR"
                auto reg = registry;
                condition = [reg,
                             negateCondition,
                             condition = getFunctionNameAndArgs(option.at(parameterIndex)),
                             this]() -> bool {
                    bool out = false;
                    if (condition.first == "quest_complete")
                    {
                        assert(!condition.second.empty());
                        auto& quest = reg->get<Quest>(gameData->questManager->GetQuest(condition.second));
                        out = quest.IsComplete();
                    }
                    else if (condition.first == "quest_in_progress")
                    {
                        assert(!condition.second.empty());
                        auto& quest = reg->get<Quest>(gameData->questManager->GetQuest(condition.second));
                        out = quest.HasStarted() && !quest.IsComplete();
                    }
                    else if (condition.first == "quest_hand_in")
                    {
                        assert(!condition.second.empty());
                        auto& quest = reg->get<Quest>(gameData->questManager->GetQuest(condition.second));
                        out = quest.HasStarted() && quest.GetTaskCompleteCount() == quest.GetTaskCount() - 1;
                    }
                    else
                    {
                        assert(0); // unknown conditional token
                    }
                    return negateCondition ? !out : out;
                };
            }
            else if (option.at(0) == "end")
            {
                assert(condition.has_value()); // ensures that 'end' has an accompanying 'if'
                condition.reset();
            }
            else if (option.size() == 2)
            {
                std::unique_ptr<dialog::Option> baseOption;
                if (condition.has_value())
                {
                    baseOption = std::make_unique<dialog::Option>(node.get(), condition.value());
                }
                else
                {
                    baseOption = std::make_unique<dialog::Option>(node.get());
                }
                baseOption->description = option.at(0);
                const auto& next = option.at(1);
                if (!next.empty() && next != "exit")
                {
                    baseOption->nextNode = next;
                }
                node->options.push_back(std::move(baseOption));
            }
            else if (option.size() == 3)
            {
                const auto& token = getFunctionNameAndArgs(option.at(0));
                if (token.first == "quest_task")
                {
                    assert(!token.second.empty());
                    auto questId = gameData->questManager->GetQuest(token.second);
                    std::unique_ptr<dialog::QuestOption> questOption;
                    if (condition.has_value())
                    {
                        questOption =
                            std::make_unique<dialog::QuestOption>(node.get(), questId, condition.value());
                    }
                    else
                    {
                        questOption = std::make_unique<dialog::QuestOption>(node.get(), questId);
                    }
                    questOption->description = option.at(1);
                    const auto& next = option.at(2);
                    if (!next.empty() && next != "exit")
                    {
                        questOption->nextNode = next;
                    }
                    node->options.push_back(std::move(questOption));
                }
                else if (token.first == "quest_start")
                {
                    assert(!token.second.empty());
                    auto questId = gameData->questManager->GetQuest(token.second);
                    std::unique_ptr<dialog::QuestStartOption> questStartOption;
                    if (condition.has_value())
                    {
                        questStartOption =
                            std::make_unique<dialog::QuestStartOption>(node.get(), questId, condition.value());
                    }
                    else
                    {
                        questStartOption = std::make_unique<dialog::QuestStartOption>(node.get(), questId);
                    }
                    questStartOption->description = option.at(1);
                    const auto& next = option.at(2);
                    if (!next.empty() && next != "exit")
                    {
                        questStartOption->nextNode = next;
                    }
                    node->options.push_back(std::move(questStartOption));
                }
                else if (token.first == "quest_finish")
                {
                    assert(!token.second.empty());
                    auto questId = gameData->questManager->GetQuest(token.second);
                    std::unique_ptr<dialog::QuestFinishOption> questFinishOption;
                    if (condition.has_value())
                    {
                        questFinishOption =
                            std::make_unique<dialog::QuestFinishOption>(node.get(), questId, condition.value());
                    }
                    else
                    {
                        questFinishOption = std::make_unique<dialog::QuestFinishOption>(node.get(), questId);
                    }

                    questFinishOption->description = option.at(1);
                    const auto& next = option.at(2);
                    if (!next.empty() && next != "exit")
                    {
                        questFinishOption->nextNode = next;
                    }
                    node->options.push_back(std::move(questFinishOption));
                }
                else
                {
                    assert(0);
                }
            }
        }
        conversation->AddNode(std::move(node));
    }

    void DialogFactory::LoadDialog()
    {
        fs::path inputPath("resources/dialog");
        if (!fs::is_directory(inputPath))
        {
            std::cout << "ERROR: DialogFactory -> Directory does not exist or path invalid." << std::endl;
            exit(1);
        }

        for (const auto& entry : fs::directory_iterator(inputPath))
        {
            if (entry.path().extension() != ".txt") continue;
            std::ifstream infile(entry.path());

            if (!infile)
            {
                std::cerr << "Could not open file: " << entry.path() << std::endl;
                continue;
            }

            std::ostringstream fileContent;
            fileContent << infile.rdbuf();
            std::string processedContent = normalizeLineEndings(preprocessDialog(fileContent.str()));
            std::stringstream contentStream(processedContent);

            entt::entity entity = entt::null;
            DialogComponent* dialogComponent = nullptr;

            std::string currentNodeName;
            std::string currentNodeContent;
            std::vector<std::vector<std::string>> currentNodeOptions;
            std::string line;

            while (std::getline(contentStream, line))
            {

                if (line == "#meta start")
                {
                    // Reset for a new dialog
                    entity = entt::null;
                    dialogComponent = nullptr;
                    currentNodeName.clear();
                    currentNodeContent.clear();
                    currentNodeOptions.clear();
                }
                else if (line.starts_with("owner:"))
                {
                    auto owner = line.substr(6);
                    owner = trim(owner);

                    entity = gameData->npcManager->GetNPC(owner);
                    dialogComponent = &registry->get<DialogComponent>(entity);

                    // Create conversation tied to this entity
                    dialogComponent->conversation = std::make_unique<dialog::Conversation>(registry, entity);
                }
                else if (line.starts_with("camera_pos:"))
                {
                    assert(dialogComponent);
                    if (entity != entt::null && registry->all_of<sgTransform>(entity))
                    {
                        auto& transform = registry->get<sgTransform>(entity);
                        std::istringstream iss(line.substr(11));
                        Vector3 filePos{0};
                        iss >> filePos.x >> filePos.y >> filePos.z;

                        // Option 1: Use file-specified position
                        // dialogComponent->conversationPos = filePos;

                        // Option 2: Relative to NPC's transform (if needed)
                        dialogComponent->conversationPos = Vector3Add(
                            transform.GetWorldPos(), Vector3Multiply(transform.forward(), {10.0f, 1, 10.0f}));
                    }
                }
                else if (line == "#node start")
                {
                    // Reset node-specific variables
                    currentNodeName.clear();
                    currentNodeContent.clear();
                    currentNodeOptions.clear();
                }
                else if (line.starts_with("title:"))
                {
                    currentNodeName = line.substr(6);
                    currentNodeName = trim(currentNodeName);
                }
                else if (line == "---")
                {
                    std::string contentLine;
                    currentNodeContent.clear();
                    while (std::getline(contentStream, contentLine) && contentLine != "---")
                    {
                        currentNodeContent += contentLine + "\n";
                    }
                }
                else if (line.starts_with("if"))
                {
                    std::istringstream iss(line);
                    std::vector<std::string> optionParts;
                    std::string part;

                    while (std::getline(iss, part, ' '))
                    {
                        part = trim(part);
                        optionParts.push_back(part);
                    }

                    currentNodeOptions.push_back(optionParts);
                }
                else if (line == "end")
                {
                    currentNodeOptions.push_back({"end"});
                }
                else if (line.starts_with("[["))
                {
                    line = line.substr(2, line.length() - 4); // Remove [[ and ]]
                    std::istringstream iss(line);
                    std::vector<std::string> optionParts;
                    std::string part;

                    while (std::getline(iss, part, '|'))
                    {
                        part = trim(part);
                        optionParts.push_back(part);
                    }

                    currentNodeOptions.push_back(optionParts);
                }
                else if (line == "#node end")
                {
                    assert(dialogComponent);
                    // Finalize and parse the node
                    if (!currentNodeName.empty())
                    {
                        parseNode(
                            dialogComponent->conversation.get(),
                            currentNodeName,
                            currentNodeContent,
                            currentNodeOptions);
                    }
                }
            }
        }
    }

    DialogFactory::DialogFactory(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }

} // namespace sage