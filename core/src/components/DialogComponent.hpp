//
// Created by steve on 11/05/2024.
//

#pragma once

#include "raylib.h"

#include "common_types.hpp"

#include <cassert>
#include <entt/entt.hpp>
#include <iostream>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace sage
{
    class Quest;

    namespace dialog
    {
        struct ConversationNode;
        class Conversation;

        class Option
        {
          public:
            std::optional<unsigned int> nextIndex;
            std::string description;
            ConversationNode* parent;
            [[nodiscard]] bool HasNextIndex();
            virtual std::variant<bool, unsigned int> GetNextIndex();
            virtual bool ShouldShow();
            virtual void OnSelected();
            virtual ~Option() = default;
            explicit Option(ConversationNode* _parent);
        };

        class ConditionalOption : public Option
        {
            std::function<bool()> condition;

          public:
            bool ShouldShow() override;
            explicit ConditionalOption(ConversationNode* _parent, std::function<bool()> _condition);
        };

        class QuestOption : public Option
        {
            bool questStart;
            entt::entity questId{};

          public:
            bool ShouldShow() override;
            void OnSelected() override;

            QuestOption(ConversationNode* _parent, entt::entity _questId, bool _questStart);
        };

        struct ConversationNode
        {
            Conversation* parent;
            unsigned int index = 0;
            std::string content;
            std::vector<std::unique_ptr<Option>> options;
            explicit ConversationNode(Conversation* _parent);
        };

        class Conversation
        {
            ConversationID conversationId{};
            unsigned int current = 0;
            std::vector<std::unique_ptr<ConversationNode>> nodes;

          public:
            entt::registry* registry;
            const entt::entity owner;
            entt::sigh<void(Conversation*)> onConversationProgress;
            entt::sigh<void()> onConversationEnd;

            [[nodiscard]] ConversationNode* GetCurrentNode() const
            {
                return nodes.at(current).get();
            }

            void SelectOption(Option* option)
            {
                option->OnSelected();
                current = option->nextIndex.value();
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

            explicit Conversation(entt::registry* _registry, entt::entity _owner)
                : registry(_registry), owner(_owner)
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
