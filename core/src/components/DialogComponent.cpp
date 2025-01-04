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

        [[nodiscard]] bool Option::HasNextIndex() const
        {
            return nextNode.has_value();
        }

        bool Option::ShouldShow()
        {
            if (condition.has_value())
            {
                return condition.value()();
            }
            return true;
        }

        void Option::OnSelected()
        {
            std::cout << "Calling base class. \n";
        }

        Option::Option(ConversationNode* _parent) : parent(_parent)
        {
        }

        Option::Option(ConversationNode* _parent, std::function<bool()> _condition)
            : condition(std::move(_condition)), parent(_parent)
        {
        }

        void QuestOption::OnSelected()
        {
            auto& task = parent->parent->registry->get<QuestTaskComponent>(parent->parent->owner);
            auto& quest = parent->parent->registry->get<Quest>(questId);
            if (quest.HasStarted())
            {
                task.MarkComplete();
            }
        }

        QuestOption::QuestOption(ConversationNode* _parent, entt::entity _questId)
            : Option(_parent), questId(_questId)
        {
        }

        QuestOption::QuestOption(
            ConversationNode* _parent, entt::entity _questId, std::function<bool()> _condition)
            : Option(_parent, std::move(_condition)), questId(_questId)
        {
        }

        void QuestStartOption::OnSelected()
        {
            auto& quest = parent->parent->registry->get<Quest>(questId);
            if (!quest.HasStarted())
            {
                quest.StartQuest();
            }
        }

        QuestStartOption::QuestStartOption(ConversationNode* _parent, entt::entity _questId)
            : QuestOption(_parent, _questId)
        {
        }

        QuestStartOption::QuestStartOption(
            ConversationNode* _parent, entt::entity _questId, std::function<bool()> _condition)
            : QuestOption(_parent, _questId, std::move(_condition))
        {
        }

        void QuestFinishOption::OnSelected()
        {
            auto& quest = parent->parent->registry->get<Quest>(questId);
            if (quest.HasStarted())
            {
                quest.CompleteQuest();
            }
        }

        QuestFinishOption::QuestFinishOption(ConversationNode* _parent, entt::entity _questId)
            : QuestOption(_parent, _questId)
        {
        }

        QuestFinishOption::QuestFinishOption(
            ConversationNode* _parent, entt::entity _questId, std::function<bool()> _condition)
            : QuestOption(_parent, _questId, std::move(_condition))
        {
        }

        ConversationNode::ConversationNode(Conversation* _parent) : parent(_parent)
        {
        }

    } // namespace dialog

} // namespace sage
