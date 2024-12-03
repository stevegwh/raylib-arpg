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
#include <sstream>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;

namespace sage
{

    void DialogFactory::parseNode(
        dialog::Conversation* conversation,
        const std::string& nodeName,
        const std::string& content,
        const std::vector<std::vector<std::string>>& optionData,
        entt::entity questId) const
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
                assert(questId != entt::null);
                auto& quest = registry->get<Quest>(questId);
                condition = [&quest, condition = option.at(1)]() -> bool {
                    if (condition == "quest_not_started")
                    {
                        return !quest.HasStarted() && !quest.IsComplete();
                    }
                    else if (condition == "quest_complete")
                    {
                        return quest.IsComplete();
                    }
                    else if (condition == "quest_in_progress")
                    {
                        return quest.HasStarted() && !quest.IsComplete();
                    }
                    else if (condition == "quest_hand_in")
                    {
                        return quest.HasStarted() && quest.GetTaskCompleteCount() == quest.GetTaskCount() - 1;
                    }
                    return true;
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
                const auto& token = option.at(0);
                // TODO: I think "ShouldShow" should not be virtual and just be controlled by the conditional
                if (token == "quest_task")
                {
                    assert(questId != entt::null);
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
                else if (token == "quest_start")
                {
                    assert(questId != entt::null);
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
                else if (token == "quest_finish")
                {
                    assert(questId != entt::null);
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
            std::ifstream infile(entry.path());
            if (!infile)
            {
                std::cerr << "Could not open file: " << entry.path() << std::endl;
                continue;
            }

            // Variables to track current parsing state
            entt::entity entity = entt::null;
            DialogComponent* dialogComponent = nullptr;

            entt::entity questId = entt::null;

            std::string currentNodeName;
            std::string currentNodeContent;
            std::vector<std::vector<std::string>> currentNodeOptions;
            std::string line;

            while (std::getline(infile, line))
            {
                // TODO: Should trim any white space from start of line, to allow for indentation.
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
                    // Trim whitespace
                    owner.erase(0, owner.find_first_not_of(' '));
                    owner.erase(owner.find_last_not_of(' ') + 1);

                    entity = gameData->npcManager->GetNPC(owner);
                    dialogComponent = &registry->get<DialogComponent>(entity);

                    // Create conversation tied to this entity
                    dialogComponent->conversation = std::make_unique<dialog::Conversation>(registry, entity);
                }
                else if (line.starts_with("camera_pos:"))
                {
                    assert(dialogComponent);
                    // Ensure entity and transform exist before accessing
                    if (entity != entt::null && registry->all_of<sgTransform>(entity))
                    {
                        auto& transform = registry->get<sgTransform>(entity);

                        // Parse camera position from file
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
                else if (line.starts_with("quest:"))
                {
                    assert(dialogComponent);
                    auto questName = line.substr(6);
                    questName.erase(0, questName.find_first_not_of(' '));
                    questName.erase(questName.find_last_not_of(' ') + 1);
                    questId = QuestManager::GetInstance().GetQuest(questName);
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
                    currentNodeName.erase(0, currentNodeName.find_first_not_of(' '));
                    currentNodeName.erase(currentNodeName.find_last_not_of(' ') + 1);
                }
                else if (line == "---")
                {
                    std::string contentLine;
                    currentNodeContent.clear();
                    while (std::getline(infile, contentLine) && contentLine != "---")
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
                        // Trim whitespace
                        part.erase(0, part.find_first_not_of(' '));
                        part.erase(part.find_last_not_of(' ') + 1);
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
                        // Trim whitespace
                        part.erase(0, part.find_first_not_of(' '));
                        part.erase(part.find_last_not_of(' ') + 1);
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
                            currentNodeOptions,
                            questId);
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