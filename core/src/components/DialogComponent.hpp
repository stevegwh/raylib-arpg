//
// Created by steve on 11/05/2024.
//

#pragma once

#include "raylib.h"
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

        struct Option
        {
            std::string description;
            std::optional<unsigned int> nextIndex;
        };

        struct ConversationNode
        {
            unsigned int index = 0;
            std::string content;
            std::vector<Option> options;
        };

        class Conversation
        {
            unsigned int current = 0;
            std::vector<std::unique_ptr<ConversationNode>> nodes;

          public:
            [[nodiscard]] ConversationNode* GetCurrentNode() const
            {
                return nodes.at(current).get();
            }

            void SelectOption(const unsigned int index)
            {
                assert(GetCurrentNode()->options[index].nextIndex.has_value());
                current = GetCurrentNode()->options[index].nextIndex.value();
            }

            explicit Conversation(std::vector<std::unique_ptr<ConversationNode>>& _nodes)
                : nodes(std::move(_nodes))
            {
            }
        };

    } // namespace dialog

    struct DialogComponent
    {
        entt::entity dialogTarget; // Who are you talking with
        Vector3 conversationPos;   // Where the other person stands
        // std::string sentence;      // tmp
        std::unique_ptr<dialog::Conversation> conversation;
    };
} // namespace sage
