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
#include "systems/PartySystem.hpp"
#include "systems/RenderSystem.hpp"

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

    struct TextFunction
    {
        std::string name;
        std::string params;
    };

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

            if (line == "<variables>")
            {
                inVariableBlock = true;
                continue;
            }

            if (line == "</variables>")
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

    TextFunction getFunctionNameAndArgs(const std::string& input)
    {
        std::string trimmedInput = trim(input);

        std::regex pattern(R"(^(\w+)\(([^)]+)\)$)");
        std::smatch match;

        if (std::regex_match(trimmedInput, match, pattern))
        {
            std::string functionName = match[1];
            std::string parameter = match[2];

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
        const std::string& dialogOptions) const
    {
        auto node = std::make_unique<dialog::ConversationNode>(conversation);
        node->title = nodeName;
        node->content = content;

        std::optional<std::function<bool()>> condition;

        std::stringstream optionStream(dialogOptions);
        std::string line;

        while (std::getline(optionStream, line, '\n'))
        {
            if (line.find("if") != std::string::npos)
            {
                assert(!condition.has_value()); // "if blocks" must be closed with end. No nesting allowed (yet).

                condition = [line, this]() -> bool {
                    std::function<bool(std::string)> quest_complete = [this](const std::string& params) {
                        auto& quest = registry->get<Quest>(gameData->questManager->GetQuest(params));
                        return quest.IsComplete();
                    };

                    std::function<bool(std::string)> quest_in_progress = [this](const std::string& params) {
                        auto& quest = registry->get<Quest>(gameData->questManager->GetQuest(params));
                        return quest.HasStarted() && !quest.IsComplete();
                        ;
                    };

                    std::function<bool(std::string)> has_item = [this](const std::string& params) {
                        return gameData->partySystem->CheckPartyHasItem(params);
                    };

                    std::function<bool(std::string)> quest_tasks_complete = [this](const std::string& params) {
                        auto& quest = registry->get<Quest>(gameData->questManager->GetQuest(params));
                        return quest.HasStarted() && quest.AllTasksComplete();
                    };

                    std::stringstream condStream(line.substr(3)); // 3 == if and initial space
                    std::string current;

                    bool out = false;
                    bool positive = true;
                    bool andCondition = false;
                    bool orCondition = false;

                    auto evaluateCondition = [&](bool condition) {
                        if (!andCondition && !orCondition)
                        {
                            out = positive && condition;
                        }
                        else if (andCondition)
                        {
                            out = out && (positive && condition);
                        }
                        else if (orCondition)
                        {
                            out = out || (positive && condition);
                        }
                    };

                    const std::unordered_map<std::string, std::function<bool(std::string)>> functionMap = {
                        {"quest_complete", quest_complete},
                        {"quest_in_progress", quest_in_progress},
                        {"has_item", has_item},
                        {"quest_tasks_complete", quest_tasks_complete}};

                    while (std::getline(condStream, current, ' '))
                    {
                        if (current == "not")
                        {
                            positive = false;
                            continue;
                        }
                        if (current == "and")
                        {
                            andCondition = true;
                            continue;
                        }
                        if (current == "or")
                        {
                            orCondition = true;
                            continue;
                        }
                        auto func = getFunctionNameAndArgs(current);
                        auto funcIt = functionMap.find(func.name);

                        if (funcIt != functionMap.end())
                        {
                            evaluateCondition(funcIt->second(func.params));
                        }
                        else
                        {
                            assert(0); // unknown conditional token
                        }

                        positive = true;
                        andCondition = false;
                        orCondition = false;
                    }
                    return out;
                };
            }
            else if (line == "end")
            {
                assert(condition.has_value()); // ensures that 'end' has an accompanying 'if'
                condition.reset();
            }
            else if (line.find("[[") != std::string::npos)
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

                std::stringstream ss(line.substr(2)); // 2 == [[
                std::string word;

                std::vector<std::string> option;

                while (std::getline(ss, word, '|'))
                {
                    option.push_back(trim(word));
                }

                if (option.size() == 2)
                {
                    baseOption->description = option.at(0);
                    const auto& next = option.at(1);
                    if (!next.empty() && next != "exit")
                    {
                        baseOption->nextNode = next;
                    }
                    node->options.push_back(std::move(baseOption));
                }
                else if (option.size() == 3) // "[[" with function
                {
                    option.at(2) = option.at(2).substr(0, option.at(2).find_first_of("]]"));

                    const auto& token = getFunctionNameAndArgs(option.at(0));
                    if (token.name == "complete_quest_task")
                    {
                        assert(!token.params.empty());
                        auto questId = gameData->questManager->GetQuest(token.params);
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
                    else if (token.name == "start_quest")
                    {
                        assert(!token.params.empty());
                        auto questId = gameData->questManager->GetQuest(token.params);
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
                    else if (token.name == "complete_quest")
                    {
                        assert(!token.params.empty());
                        auto questId = gameData->questManager->GetQuest(token.params);
                        std::unique_ptr<dialog::QuestFinishOption> questFinishOption;
                        if (condition.has_value())
                        {
                            questFinishOption = std::make_unique<dialog::QuestFinishOption>(
                                node.get(), questId, condition.value());
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
            std::string currentNodeSpeakerText;
            std::string currentNodeOptions;
            std::string line;

            while (std::getline(contentStream, line))
            {
                if (line == "<meta>")
                {
                    // Reset for a new dialog
                    entity = entt::null;
                    dialogComponent = nullptr;
                    currentNodeName.clear();
                    currentNodeSpeakerText.clear();
                    currentNodeOptions.clear();
                }
                else if (line.starts_with("owner:"))
                {
                    auto owner = line.substr(std::string("owner:").size());
                    owner = trim(owner);

                    entity = gameData->renderSystem->FindRenderableByName<DialogComponent>(owner);
                    if (entity == entt::null)
                    {
                        entity = gameData->renderSystem->FindRenderableByName(owner);
                    }
                    assert(entity != entt::null);
                    dialogComponent = &registry->get<DialogComponent>(entity);

                    // Create conversation tied to this entity
                    dialogComponent->conversation = std::make_unique<dialog::Conversation>(registry, entity);
                }
                else if (line.starts_with("speaker_name:"))
                {
                    assert(dialogComponent);
                    auto str_size = std::string("speaker_name:").size();
                    auto speaker = line.substr(str_size);
                    speaker = trim(speaker);
                    dialogComponent->conversation->speaker = speaker;
                }
                else if (line.starts_with("conversation_pos:"))
                {
                    assert(dialogComponent);
                    if (entity != entt::null && registry->all_of<sgTransform>(entity))
                    {
                        auto& transform = registry->get<sgTransform>(entity);
                        auto str_size = std::string("conversation_pos:").size();
                        std::istringstream iss(line.substr(str_size));
                        Vector3 filePos{0};
                        iss >> filePos.x >> filePos.y >> filePos.z;
                        dialogComponent->conversationPos =
                            Vector3Add(transform.GetWorldPos(), Vector3Multiply(transform.forward(), filePos));
                        ;
                    }
                }
                else if (line == "<node>")
                {
                    // Reset node-specific variables
                    currentNodeName.clear();
                    currentNodeSpeakerText.clear();
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
                    currentNodeSpeakerText.clear();
                    while (std::getline(contentStream, contentLine) && contentLine != "---")
                    {
                        currentNodeSpeakerText += contentLine + "\n";
                    }
                }
                else if (line.starts_with("if"))
                {
                    currentNodeOptions += (trim(line) + "\n");
                }
                else if (line == "end")
                {
                    currentNodeOptions += "end\n";
                }
                else if (line.starts_with("[["))
                {
                    currentNodeOptions += (trim(line) + "\n");
                }
                else if (line == "</node>")
                {
                    assert(dialogComponent);
                    // Finalize and parse the node
                    if (!currentNodeName.empty())
                    {
                        parseNode(
                            dialogComponent->conversation.get(),
                            currentNodeName,
                            currentNodeSpeakerText,
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