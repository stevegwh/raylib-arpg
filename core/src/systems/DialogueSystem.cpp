//
// Created by steve on 11/05/2024.
//

#include "DialogueSystem.hpp"

#include "Camera.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"

#include "components/Animation.hpp"
#include "components/Collideable.hpp"
#include "components/DialogComponent.hpp"
#include "components/MoveableActor.hpp"
#include "components/sgTransform.hpp"
#include "GameUiElements.hpp"
#include "systems/ControllableActorSystem.hpp"

#include "raylib.h"
#include "raymath.h"

#include <iostream>

namespace sage
{
    void DialogueSystem::changeControlledActor(entt::entity entity)
    {
        selectedActor = entity;
    }

    void DialogueSystem::startConversation(entt::entity entity)
    {
        onConversationStart.publish();

        const auto& npcDiag = registry->get<DialogComponent>(clickedNPC);
        std::cout << npcDiag.sentence << std::endl;
        registry->get<Animation>(clickedNPC).ChangeAnimation(1); // TODO: Change to an enum

        // Rotate to look at NPC
        auto& actorTrans = registry->get<sgTransform>(selectedActor);
        auto& npcTrans = registry->get<sgTransform>(clickedNPC);
        Vector3 direction = Vector3Subtract(npcTrans.GetWorldPos(), actorTrans.GetWorldPos());
        direction = Vector3Normalize(direction);
        float angle = atan2f(direction.x, direction.z);
        actorTrans.SetRotation({actorTrans.GetWorldRot().x, RAD2DEG * angle, actorTrans.GetWorldRot().z});

        {
            entt::sink sink{gameData->cursor->onAnyLeftClick};
            sink.connect<&DialogueSystem::endConversation>(this);
        }

        oldCamPos = gameData->camera->GetPosition();
        oldCamTarget = gameData->camera->getRaylibCam()->target;
        gameData->camera->CutscenePose(npcTrans);
        gameData->camera->LockInput();

        gameData->cursor->DisableContextSwitching();
        // gameData->controllableActorSystem->Disable();
        // TODO: Switch to dialogue state. Maybe trigger an onDialogueStart event?
        active = true;

        {
            auto& moveableActor = registry->get<MoveableActor>(selectedActor);
            entt::sink sink{moveableActor.onFinishMovement};
            sink.disconnect<&DialogueSystem::startConversation>(this);
            entt::sink sink2{moveableActor.onMovementCancel};
            sink2.disconnect<&DialogueSystem::cancelConversation>(this);
        }
    }

    void DialogueSystem::endConversation(entt::entity actor)
    {
        onConversationEnd.publish();
        {
            entt::sink sink{gameData->cursor->onAnyLeftClick};
            sink.disconnect<&DialogueSystem::endConversation>(this);
        }

        gameData->camera->UnlockInput();
        gameData->cursor->EnableContextSwitching();
        // gameData->controllableActorSystem->Enable();
        // TODO: Trigger an onDialogueEnd event which changes player state.
        active = false;
        registry->get<Animation>(clickedNPC).ChangeAnimation(0); // TODO: Change to an enum
        clickedNPC = entt::null;

        gameData->camera->SetCamera(oldCamPos, oldCamTarget);
        oldCamPos = {};
        oldCamTarget = {};
    }

    void DialogueSystem::cancelConversation(entt::entity entity)
    {
        auto& moveableActor = registry->get<MoveableActor>(selectedActor);
        entt::sink sink{moveableActor.onFinishMovement};
        sink.disconnect<&DialogueSystem::startConversation>(this);
        entt::sink sink2{moveableActor.onMovementCancel};
        sink2.disconnect<&DialogueSystem::cancelConversation>(this);
        clickedNPC = entt::null;
    }

    void DialogueSystem::NPCClicked(entt::entity _clickedNPC)
    {
        if (clickedNPC != entt::null) return;
        clickedNPC = _clickedNPC;
        const auto& npc = registry->get<DialogComponent>(_clickedNPC);
        const auto& actorCol = registry->get<Collideable>(selectedActor);
        const auto& npcCol = registry->get<Collideable>(_clickedNPC);
        gameData->controllableActorSystem->PathfindToLocation(selectedActor, npc.conversationPos);
        {
            auto& moveableActor = registry->get<MoveableActor>(selectedActor);
            entt::sink sink{moveableActor.onFinishMovement};
            sink.connect<&DialogueSystem::startConversation>(this);
            entt::sink sink2{moveableActor.onMovementCancel};
            sink2.connect<&DialogueSystem::cancelConversation>(this);
        }
    }

    DialogueSystem::DialogueSystem(entt::registry* registry, GameData* _gameData)
        : BaseSystem(registry), clickedNPC(entt::null), gameData(_gameData)
    {
        {
            entt::sink sink{gameData->controllableActorSystem->onSelectedActorChange};
            sink.connect<&DialogueSystem::changeControlledActor>(this);
            selectedActor = gameData->controllableActorSystem->GetSelectedActor();
        }
        {
            entt::sink sink{gameData->cursor->onNPCClick};
            sink.connect<&DialogueSystem::NPCClicked>(this);
        }
    }
} // namespace sage
