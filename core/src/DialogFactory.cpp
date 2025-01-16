//
// Created by Steve Wheeler on 29/11/2024.
//

#include "DialogFactory.hpp"

#include "components/DialogComponent.hpp"
#include "components/QuestComponents.hpp"
#include "components/sgTransform.hpp"
#include "NpcManager.hpp"
#include "ParsingHelpers.hpp"
#include "QuestManager.hpp"
#include "Systems.hpp"
#include "systems/RenderSystem.hpp"

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
    using namespace parsing;

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

    static std::string substituteVariablesInText(const std::string& content)
    {
        auto variables = extractVariables(content);
        std::string result = content;

        for (const auto& [varName, value] : variables)
        {
            result = std::regex_replace(result, std::regex(R"(\$)" + varName), value);
        }

        return result;
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
            if (line.starts_with("if"))
            {
                assert(!condition.has_value()); // "if blocks" must be closed with end. No nesting allowed (yet).

                condition = GetConditionalStatement(line, registry, sys);
            }
            else if (line == "end")
            {
                assert(condition.has_value()); // ensures that 'end' has an accompanying 'if'
                condition.reset();
            }
            else if (line.starts_with("[["))
            {
                std::stringstream ss(line.substr(2)); // 2 == [[
                std::string word;
                std::vector<std::string> option;

                while (std::getline(ss, word, '|'))
                {
                    option.push_back(parsing::trim(word));
                }

                auto& lastWord = option.at(option.size() - 1);
                lastWord = lastWord.substr(0, lastWord.find_first_of("]]"));

                if (option.size() == 2)
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
                    if (const auto& next = option.at(1); !next.empty() && next != "exit")
                    {
                        baseOption->nextNode = next;
                    }
                    node->options.push_back(std::move(baseOption));
                }
                else if (option.size() == 3) // "[[" with function
                {
                    const auto& func = getFunctionNameAndArgs(option.at(0));
                    if (func.name == "complete_quest_task")
                    {
                        assert(!func.params.empty());
                        auto questId = sys->questManager->GetQuest(func.params);
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
                    else if (func.name == "start_quest")
                    {
                        assert(!func.params.empty());
                        auto questId = sys->questManager->GetQuest(func.params);
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
                    else if (func.name == "complete_quest")
                    {
                        assert(!func.params.empty());
                        auto questId = sys->questManager->GetQuest(func.params);
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

    void DialogFactory::InitDialogFromDirectory()
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
            std::string processedContent = trimWhiteSpaceFromFile(
                substituteVariablesInText(removeCommentsFromFile(normalizeLineEndings(fileContent.str()))));
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

                    entity = sys->renderSystem->FindRenderable<DialogComponent>(owner);
                    assert(entity != entt::null);
                    // if (!registry->any_of<DialogComponent>(entity))
                    // {
                    //     registry->emplace<DialogComponent>(entity);
                    // }
                    dialogComponent = &registry->get<DialogComponent>(entity);
                    dialogComponent->conversation = std::make_unique<dialog::Conversation>(registry, sys, entity);
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
                        dialogComponent->conversationPos = Vector3Add(
                            transform.GetWorldPos(),
                            Vector3Multiply(
                                transform.forward(), filePos)); // This doesn't account for movable NPCs at all
                    }
                }
                else if (line.starts_with("camera_pos:"))
                {
                    assert(dialogComponent);
                    if (entity != entt::null && registry->all_of<sgTransform>(entity))
                    {
                        auto& transform = registry->get<sgTransform>(entity);
                        auto str_size = std::string("camera_pos:").size();
                        std::istringstream iss(line.substr(str_size));
                        Vector3 filePos{0};
                        iss >> filePos.x >> filePos.y >> filePos.z;
                        dialogComponent->cameraPos = filePos;
                    }
                }
                else if (line == "<node>")
                {
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

    DialogFactory::DialogFactory(entt::registry* _registry, Systems* _sys) : registry(_registry), sys(_sys)
    {
    }

} // namespace sage