//
// Created by steve on 11/05/2024.
//

#pragma once

#include "raylib.h"

#include "common_types.hpp"

#include <cassert>
#include <entt/entt.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace sage
{

    namespace dialog
    {
        struct ConversationNode;
        class Conversation;

        struct Option
        {
            ConversationNode* parent;
            std::string description;
            std::optional<unsigned int> nextIndex;
            explicit Option(ConversationNode* _parent) : parent(_parent)
            {
            }
        };

        struct QuestOption : public Option
        {
            entt::entity questId{};

            QuestOption(ConversationNode* _parent, entt::entity _questId) : Option(_parent), questId(_questId)
            {
            }
        };

        struct ConversationNode
        {
            Conversation* parent;
            unsigned int index = 0;
            std::string content;
            std::vector<Option> options;
            explicit ConversationNode(Conversation* _parent) : parent(_parent)
            {
            }
        };

        class Conversation
        {
            ConversationID conversationId{};
            unsigned int current = 0;
            std::vector<std::unique_ptr<ConversationNode>> nodes;

          public:
            const entt::entity owner;
            entt::sigh<void(Conversation*)> onConversationProgress;
            entt::sigh<void()> onConversationEnd;
            entt::sigh<void()> onQuestAccepted; // TODO: Need to pass information about the quest to the manager

            [[nodiscard]] ConversationNode* GetCurrentNode() const
            {
                return nodes.at(current).get();
            }

            void SelectOption(const unsigned int index)
            {
                assert(GetCurrentNode()->options[index].nextIndex.has_value());
                current = GetCurrentNode()->options[index].nextIndex.value();
                onConversationProgress.publish(this);
            }

            void EndConversation()
            {
                current = 0;
                onConversationEnd.publish();
            }

            void AddNode(std::unique_ptr<ConversationNode> node)
            {
                nodes.push_back(std::move(node));
            }

            explicit Conversation(entt::entity _owner) : owner(_owner)
            {
            }
        };

    } // namespace dialog

    struct DialogComponent
    {
        entt::entity dialogTarget; // Who are you talking with
        Vector3 conversationPos;   // Where the other person stands
        std::unique_ptr<dialog::Conversation> conversation;
    };
} // namespace sage
