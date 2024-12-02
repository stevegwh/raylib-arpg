//
// Created by Steve Wheeler on 29/11/2024.
//

#include "DialogFactory.hpp"

#include "components/DialogComponent.hpp"
#include "components/QuestComponents.hpp"
#include "QuestManager.hpp"

namespace sage
{

    void GetKnightDialog(entt::registry* registry, entt::entity entity)
    {
        auto& dialog = registry->get<DialogComponent>(entity);
        auto questId = QuestManager::GetInstance().GetQuest("Test Quest");

        {
            auto node = std::make_unique<dialog::ConversationNode>(dialog.conversation.get());
            node->content = "Hello! \n";
            node->title = "start";

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
            option3->nextNode = "exit";

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
            node->content = "Sure, talk to my friend! \n";
            node->title = "quest";

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
            node->content = "Ok! \n";
            node->title = "exit";
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

    DialogFactory::DialogFactory(entt::registry* _registry) : registry(_registry)
    {
    }

} // namespace sage