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

    void GetKnightDialog(entt::registry* registry, entt::entity entity)
    {
        auto& dialog = registry->get<DialogComponent>(entity);
        auto questId = QuestManager::GetInstance().GetQuest("Test Quest");

        {
            auto node = std::make_unique<dialog::ConversationNode>(dialog.conversation.get());
            node->title = "start";
            node->content = "Hello! \n";

            auto option1 = std::make_unique<dialog::ConditionalOption>(node.get(), [registry, questId]() {
                auto& quest = registry->get<Quest>(questId);
                return !quest.HasStarted() && !quest.IsComplete();
            });
            option1->description = "Do you have any quests for me? \n";
            option1->nextNode = "quest";

            auto option3 = std::make_unique<dialog::ConditionalOption>(node.get(), [registry, questId]() {
                auto& quest = registry->get<Quest>(questId);
                return quest.IsComplete();
            });
            option3->description = "I completed the quest! \n";
            option3->nextNode = "quest complete";

            auto option2 = std::make_unique<dialog::Option>(node.get());
            option2->description = "Sorry, I must be leaving. \n";
            option2->nextNode = "exit";

            node->options.push_back(std::move(option1));
            node->options.push_back(std::move(option3));
            node->options.push_back(std::move(option2));
            dialog.conversation->AddNode(std::move(node));
        }

        {
            auto node = std::make_unique<dialog::ConversationNode>(dialog.conversation.get());
            node->title = "quest";
            node->content = "Sure, talk to my friend! \n";

            auto option1 = std::make_unique<dialog::QuestStartOption>(node.get(), questId);
            option1->description = "Ok, sure. \n";
            option1->nextNode = "exit";

            auto option2 = std::make_unique<dialog::Option>(node.get());
            option2->description = "No, thank you! \n";
            option2->nextNode = "exit";

            node->options.push_back(std::move(option1));
            node->options.push_back(std::move(option2));
            dialog.conversation->AddNode(std::move(node));
        }
        {
            auto node = std::make_unique<dialog::ConversationNode>(dialog.conversation.get());
            node->title = "exit";
            node->content = "Ok! \n";
            auto option1 = std::make_unique<dialog::Option>(node.get());
            option1->description = "Take your leave \n";
            node->options.push_back(std::move(option1));
            dialog.conversation->AddNode(std::move(node));
        }
    }

    void GetFetchQuestNPCDialog(entt::registry* registry, entt::entity entity)
    {
        auto& dialog = registry->get<DialogComponent>(entity);
        auto questId = QuestManager::GetInstance().GetQuest("Item Fetch Quest");
        {
            auto node = std::make_unique<dialog::ConversationNode>(dialog.conversation.get());
            node->content = "Hello there, how can I help? \n";
            node->title = "start";
            auto option1 = std::make_unique<dialog::QuestHandInOption>(node.get(), questId);
            option1->description = "I found this item on the ground, is it yours? \n";
            option1->nextNode = "quest";
            auto option2 = std::make_unique<dialog::Option>(node.get());
            option2->description = "Take your leave. \n";
            option2->nextNode = "exit";
            node->options.push_back(std::move(option1));
            node->options.push_back(std::move(option2));
            dialog.conversation->AddNode(std::move(node));
        }
        {
            auto node = std::make_unique<dialog::ConversationNode>(dialog.conversation.get());
            node->content = "Oh, thank you! I've been looking everywhere for this. \n";
            node->title = "quest";
            auto option1 = std::make_unique<dialog::Option>(node.get());
            option1->description = "Take your leave \n";
            node->options.push_back(std::move(option1));
            dialog.conversation->AddNode(std::move(node));
        }
        {
            auto node = std::make_unique<dialog::ConversationNode>(dialog.conversation.get());
            node->content = "Good day. \n";
            node->title = "exit";
            auto option1 = std::make_unique<dialog::Option>(node.get());
            option1->description = "Take your leave \n";
            node->options.push_back(std::move(option1));
            dialog.conversation->AddNode(std::move(node));
        }
        // -------------------------------
    }

    void GetQuestNPCDialog(entt::registry* registry, entt::entity entity)
    {
        auto& dialog = registry->get<DialogComponent>(entity);

        auto questId = QuestManager::GetInstance().GetQuest("Test Quest");
        {
            auto node = std::make_unique<dialog::ConversationNode>(dialog.conversation.get());
            node->content = "Hello there, how can I help? \n";
            node->title = "start";
            auto option1 = std::make_unique<dialog::QuestHandInOption>(node.get(), questId);
            option1->description = "I want to complete the quest. \n";
            option1->nextNode = "quest";
            auto option2 = std::make_unique<dialog::Option>(node.get());
            option2->description = "Hello! \n";
            option2->nextNode = "exit";
            node->options.push_back(std::move(option1));
            node->options.push_back(std::move(option2));
            dialog.conversation->AddNode(std::move(node));
        }
        {
            auto node = std::make_unique<dialog::ConversationNode>(dialog.conversation.get());
            node->content = "Quest complete! \n";
            node->title = "quest";
            auto option1 = std::make_unique<dialog::Option>(node.get());
            option1->description = "Take your leave \n";
            node->options.push_back(std::move(option1));
            dialog.conversation->AddNode(std::move(node));
        }
        {
            auto node = std::make_unique<dialog::ConversationNode>(dialog.conversation.get());
            node->content = "Hello! \n";
            node->title = "exit";
            auto option1 = std::make_unique<dialog::Option>(node.get());
            option1->description = "Take your leave \n";
            node->options.push_back(std::move(option1));
            dialog.conversation->AddNode(std::move(node));
        }
        // -------------------------------
    }

    void DialogFactory::GetDialog(const std::string& npcName, entt::entity entity) const
    {
        if (npcName == "Knight")
        {
            GetKnightDialog(registry, entity);
            return;
        }
        else if (npcName == "Fetch Quest NPC")
        {
            GetFetchQuestNPCDialog(registry, entity);
            return;
        }
        else if (npcName == "Quest NPC")
        {
            GetQuestNPCDialog(registry, entity);
            return;
        }
        assert(0);
    }

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

        for (const auto& option : optionData)
        {
            if (option.size() == 2)
            {
                auto baseOption = std::make_unique<dialog::Option>(node.get());
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

                if (token == "quest_start")
                {
                    auto questStartOption = std::make_unique<dialog::QuestStartOption>(node.get(), questId);
                    questStartOption->description = option.at(1);
                    const auto& next = option.at(2);
                    if (!next.empty() && next != "exit")
                    {
                        questStartOption->nextNode = next;
                    }
                    node->options.push_back(std::move(questStartOption));
                    // TODO: this does not start the quest
                }
            }
            else if (option.size() > 3)
            {
                // TODO: check if "quest" related
                auto& quest = registry->get<Quest>(questId);
                auto conditionalOption = std::make_unique<dialog::ConditionalOption>(
                    node.get(), [&quest, condition = option.at(1)]() -> bool {
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
                        return true;
                    });
                conditionalOption->description = option.at(2);
                const auto& next = option.at(3);
                if (!next.empty() && next != "exit")
                {
                    conditionalOption->nextNode = next;
                }
                node->options.push_back(std::move(conditionalOption));
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

            auto questId = QuestManager::GetInstance().GetQuest("Test Quest");

            std::string currentNodeName;
            std::string currentNodeContent;
            std::vector<std::vector<std::string>> currentNodeOptions;
            std::string line;

            while (std::getline(infile, line))
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