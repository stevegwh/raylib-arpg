//
// Created by steve on 11/05/2024.
//

#include "DialogueSystem.hpp"

#include "components/Animation.hpp"
#include "components/Collideable.hpp"
#include "components/Dialogue.hpp"
#include "components/MovableActor.hpp"
#include "components/sgTransform.hpp"
#include "GameData.hpp"

#include "raylib.h"
#include "raymath.h"

#include <iostream>

namespace sage
{
    void DialogueSystem::changeControlledActor(entt::entity entity)
    {
        controlledActor = entity;
    }

    void DialogueSystem::startConversation(entt::entity entity)
    {
        onConversationStart.publish();

        const auto& npcDiag = registry->get<Dialogue>(clickedNPC);
        std::cout << npcDiag.sentence << std::endl;
        registry->get<Animation>(clickedNPC)
            .ChangeAnimation(1); // TODO: Change to an enum

        // Rotate to look at NPC
        auto& actorTrans = registry->get<sgTransform>(controlledActor);
        auto& npcTrans = registry->get<sgTransform>(clickedNPC);
        Vector3 direction = Vector3Subtract(npcTrans.position(), actorTrans.position());
        direction = Vector3Normalize(direction);
        float angle = atan2f(direction.x, direction.z);
        actorTrans.SetRotation(
            {actorTrans.rotation().x, RAD2DEG * angle, actorTrans.rotation().z},
            controlledActor);

        {
            entt::sink sink{gameData->cursor->onAnyClick};
            sink.connect<&DialogueSystem::endConversation>(this);
        }

        oldCamPos = gameData->camera->getRaylibCam()->position;
        oldCamTarget = gameData->camera->getRaylibCam()->target;

        gameData->camera->CutscenePose(npcTrans);
        gameData->camera->LockInput();

        gameData->cursor->DisableContextSwitching();
        // gameData->controllableActorSystem->Disable();
        // TODO: Switch to dialogue state. Maybe trigger an onDialogueStart event?
        active = true;

        {
            auto& moveableActor = registry->get<MoveableActor>(controlledActor);
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
            entt::sink sink{gameData->cursor->onAnyClick};
            sink.disconnect<&DialogueSystem::endConversation>(this);
        }

        gameData->camera->UnlockInput();
        gameData->cursor->EnableContextSwitching();
        // gameData->controllableActorSystem->Enable();
        // TODO: Trigger an onDialogueEnd event which changes player state.
        active = false;
        registry->get<Animation>(clickedNPC)
            .ChangeAnimation(0); // TODO: Change to an enum
        clickedNPC = entt::null;

        gameData->camera->SetCamera(oldCamPos, oldCamTarget);
        oldCamPos = {};
        oldCamTarget = {};
    }

    void DialogueSystem::cancelConversation(entt::entity entity)
    {
        auto& moveableActor = registry->get<MoveableActor>(controlledActor);
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
        const auto& npc = registry->get<Dialogue>(_clickedNPC);
        const auto& actorCol = registry->get<Collideable>(controlledActor);
        const auto& npcCol = registry->get<Collideable>(_clickedNPC);
        gameData->controllableActorSystem->PathfindToLocation(
            controlledActor, npc.conversationPos);
        {
            auto& moveableActor = registry->get<MoveableActor>(controlledActor);
            entt::sink sink{moveableActor.onFinishMovement};
            sink.connect<&DialogueSystem::startConversation>(this);
            entt::sink sink2{moveableActor.onMovementCancel};
            sink2.connect<&DialogueSystem::cancelConversation>(this);
        }
    }

    void DialogueSystem::Update()
    {
        if (!active) return;
    }

    void DialogueSystem::Draw2D()
    {
        if (!active) return;
        window->Draw();
    }

    DialogueSystem::DialogueSystem(entt::registry* registry, GameData* _gameData)
        : BaseSystem(registry),
          clickedNPC(entt::null),
          gameData(_gameData),
          window(std::make_unique<DialogueWindow>(gameData->settings))
    {
        {
            entt::sink sink{gameData->controllableActorSystem->onControlledActorChange};
            sink.connect<&DialogueSystem::changeControlledActor>(this);
            controlledActor = gameData->controllableActorSystem->GetControlledActor();
        }
        {
            entt::sink sink{gameData->cursor->onNPCClick};
            sink.connect<&DialogueSystem::NPCClicked>(this);
        }
    }
} // namespace sage
