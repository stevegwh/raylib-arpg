//
// Created by steve on 11/05/2024.
//

#pragma once

#include "Systems.hpp"
#include "UserInput.hpp"

#include "raylib.h"

#include "common_types.hpp"

#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/core/type_traits.hpp"
#include "raylib-cereal.hpp"
#include "sage-cereal.hpp"
#include <cereal/archives/json.hpp>

#include "entt/entt.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <unordered_map>
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
          protected:
            std::optional<std::function<bool()>> condition;

          public:
            std::optional<std::string> nextNode;
            // std::optional<unsigned int> nextNode;
            std::string description;
            ConversationNode* parent;
            [[nodiscard]] bool HasNextIndex() const;
            bool ShouldShow() const;
            virtual void OnSelected();

            virtual ~Option() = default;
            explicit Option(ConversationNode* _parent);
            Option(ConversationNode* _parent, std::function<bool()> _condition);
        };

        // Shows if quest has started. Marks as complete. Does not finish the quest.
        class QuestOption : public Option
        {
          protected:
            entt::entity questId{};

          public:
            void OnSelected() override;

            QuestOption(ConversationNode* _parent, entt::entity _questId);
            QuestOption(ConversationNode* _parent, entt::entity _questId, std::function<bool()> _condition);
        };

        // Shows if quest has not been started (and starts quest on select)
        class QuestStartOption : public QuestOption
        {

          public:
            void OnSelected() override;

            QuestStartOption(ConversationNode* _parent, entt::entity _questId);
            QuestStartOption(ConversationNode* _parent, entt::entity _questId, std::function<bool()> _condition);
        };

        // Shows if the dialog task is the final task left to be complete
        class QuestFinishOption : public QuestOption
        {

          public:
            void OnSelected() override;
            QuestFinishOption(ConversationNode* _parent, entt::entity _questId);
            QuestFinishOption(ConversationNode* _parent, entt::entity _questId, std::function<bool()> _condition);
        };

        struct ConversationNode
        {
            Conversation* parent;
            // unsigned int index = 0;
            std::string title;
            std::string content;
            std::vector<std::unique_ptr<Option>> options;

            template <class Archive>
            void save(Archive& archive) const
            {
                archive();
            }

            template <class Archive>
            void load(Archive& archive)
            {
                archive();
            }

            explicit ConversationNode(Conversation* _parent);
        };

        class Conversation
        {
            Systems* sys;
            ConversationID conversationId{};
            // unsigned int current = 0;
            std::string start;
            std::string current;
            std::vector<std::unique_ptr<Connection>> optionPressedConnections;

            std::unordered_map<std::string, std::unique_ptr<ConversationNode>> nodes;

            void endConversation()
            {
                current = start;
                onConversationEnd.Publish();
            }

          public:
            entt::registry* registry;
            const entt::entity owner;
            std::string speaker{}; // Name of speaker to appear in text (uses owner renderable name if none)
            Event<Conversation*> onConversationProgress;
            Event<> onConversationEnd;

            [[nodiscard]] ConversationNode* GetCurrentNode() const
            {
                return nodes.at(current).get();
            }

            void BindKeysToOptionSelect()
            {
                for (const auto& cnx : optionPressedConnections)
                {
                    cnx->UnSubscribe();
                }
                optionPressedConnections.clear();
                auto& n = nodes.at(current);
                for (unsigned int i = 0; i < n->options.size(); ++i)
                {
                    auto& o = n->options.at(i);
                    if (i == 0)
                    {
                        optionPressedConnections.push_back(
                            sys->userInput->keyOnePressed.Subscribe([&o, this] { SelectOption(o.get()); }));
                    }
                    else if (i == 1)
                    {
                        optionPressedConnections.push_back(
                            sys->userInput->keyTwoPressed.Subscribe([&o, this] { SelectOption(o.get()); }));
                    }
                    else if (i == 2)
                    {
                        optionPressedConnections.push_back(
                            sys->userInput->keyThreePressed.Subscribe([&o, this] { SelectOption(o.get()); }));
                    }
                    else if (i == 3)
                    {
                        optionPressedConnections.push_back(
                            sys->userInput->keyFourPressed.Subscribe([&o, this] { SelectOption(o.get()); }));
                    }
                    else
                    {
                        assert(0);
                        // TODO: Need to have a better way of doing this, also to have more numbers bound
                        // than 4.
                    }
                }
            }

            void SelectOption(Option* option)
            {
                option->OnSelected();
                if (option->HasNextIndex())
                {
                    current = option->nextNode.value();
                    BindKeysToOptionSelect();
                    onConversationProgress.Publish(this);
                }
                else
                {
                    for (const auto& cnx : optionPressedConnections)
                    {
                        cnx->UnSubscribe();
                    }
                    optionPressedConnections.clear();
                    endConversation();
                }
            }

            void AddNode(std::unique_ptr<ConversationNode> node)
            {
                if (nodes.empty())
                {
                    start = node->title;
                    current = start;
                }
                nodes.emplace(node->title, std::move(node));
            }

            template <class Archive>
            void save(Archive& archive) const
            {
                archive();
            }

            template <class Archive>
            void load(Archive& archive)
            {
                archive();
            }

            explicit Conversation(entt::registry* _registry, Systems* _sys, entt::entity _owner)
                : registry(_registry), sys(_sys), owner(_owner)
            {
            }
        };

    } // namespace dialog

    struct DialogComponent
    {
        entt::entity dialogTarget;        // Who are you talking with
        Vector3 conversationPos;          // Where the other person stands
        std::optional<Vector3> cameraPos; // The *local* camera position during the conversation (does not
                                          // reposition camera if not set)
        std::unique_ptr<dialog::Conversation> conversation;

        template <class Archive>
        void save(Archive& archive) const
        {
            archive();
        }

        template <class Archive>
        void load(Archive& archive)
        {
            archive();
        }
    };
} // namespace sage
