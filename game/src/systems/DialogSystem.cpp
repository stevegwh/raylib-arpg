//
// Created by steve on 11/05/2024.
//

#include "DialogSystem.hpp"

#include "ControllableActorSystem.hpp"
#include "engine/Camera.hpp"
#include "engine/GameUiEngine.hpp"

#include "engine/Cursor.hpp"
#include "Systems.hpp"
#include "ui/GameUiFactory.hpp"

namespace lq
{

    void DialogSystem::progressConversation(const dialog::Conversation* conversation)
    {
        dialogWindow->Remove();
        dialogWindow = GameUiFactory::CreateDialogWindow(&sys->UI(), conversation->owner);
    }

    void DialogSystem::StartConversation(const sage::sgTransform& cutscenePose, entt::entity npc)
    {
        const auto& dialogComponent = registry->get<DialogComponent>(npc);
        if (dialogComponent.cameraPos.has_value())
        {
            sys->engine.camera->CutscenePose(cutscenePose, dialogComponent.cameraPos.value());
        }
        sys->engine.camera->LockInput();
        sys->engine.cursor->DisableContextSwitching();

        dialogComponent.conversation->BindKeysToOptionSelect();

        dialogComponent.conversation->onConversationProgress.Subscribe(
            [this](const dialog::Conversation* conv) { progressConversation(conv); });

        dialogComponent.conversation->onConversationEnd.Subscribe([this, npc]() { endConversation(npc); });

        dialogWindow = GameUiFactory::CreateDialogWindow(&sys->UI(), npc);
    }

    void DialogSystem::endConversation(entt::entity npc) const
    {
        sys->engine.camera->UnlockInput();
        sys->engine.cursor->EnableContextSwitching();
        if (const auto& dialogComponent = registry->get<DialogComponent>(npc);
            dialogComponent.cameraPos.has_value())
        {
            sys->engine.camera->CutsceneEnd();
        }
        dialogWindow->Remove();
    }

    dialog::Conversation* DialogSystem::GetConversation(entt::entity owner, ConversationID conversationId)
    {
        return nullptr;
    }

    DialogSystem::DialogSystem(entt::registry* registry, Systems* _sys) : registry(registry), sys(_sys)
    {
    }
} // namespace lq
