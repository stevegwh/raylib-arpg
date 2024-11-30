//
// Created by steve on 11/05/2024.
//

#include "DialogComponent.hpp"

#include <utility>

#include "QuestComponents.hpp"

namespace sage
{
    namespace dialog
    {

        [[nodiscard]] bool Option::HasNextIndex()
        {
            return nextIndex.has_value();
        }

        std::variant<bool, unsigned int> Option::GetNextIndex()
        {
            return nextIndex.value_or(false);
        }

        bool Option::ShouldShow()
        {
            return true;
        }

        void Option::OnSelected()
        {
            std::cout << "Calling base class. \n";
        }

        Option::Option(ConversationNode* _parent) : parent(_parent)
        {
        }

        bool ConditionalOption::ShouldShow()
        {
            return condition();
        }

        ConditionalOption::ConditionalOption(ConversationNode* _parent, std::function<bool()> _condition)
            : Option(_parent), condition(std::move(_condition))
        {
        }

        bool QuestOption::ShouldShow()
        {
            auto& quest = parent->parent->registry->get<Quest>(questId);
            // if (quest.IsComplete()) return false;
            if (questStart)
            {
                return !quest.HasStarted();
            }
            return quest.HasStarted();
        }

        void QuestOption::OnSelected()
        {
            // Could/Should just fire events here and let Quest handle it
            if (questStart)
            {
                auto& quest = parent->parent->registry->get<Quest>(questId);
                if (!quest.HasStarted())
                {
                    quest.StartQuest();
                }
            }
            else
            {
                auto& task = parent->parent->registry->get<QuestTaskComponent>(parent->parent->owner);
                auto& quest = parent->parent->registry->get<Quest>(questId);
                if (quest.HasStarted())
                {
                    task.MarkComplete();
                }
            }
        }

        QuestOption::QuestOption(ConversationNode* _parent, entt::entity _questId, bool _questStart)
            : Option(_parent), questStart(_questStart), questId(_questId)
        {
        }

        ConversationNode::ConversationNode(Conversation* _parent) : parent(_parent)
        {
        }

    } // namespace dialog

} // namespace sage
