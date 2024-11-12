//
// Created by steve on 11/05/2024.
//

#include "DialogSystem.hpp"

#include "Camera.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"

#include "components/Animation.hpp"
#include "components/Collideable.hpp"
#include "components/DialogComponent.hpp"
#include "components/MoveableActor.hpp"
#include "components/sgTransform.hpp"
#include "systems/ControllableActorSystem.hpp"

#include "raylib.h"
#include "raymath.h"

#include <iostream>

namespace sage
{

    // void DialogSystem::startConversation(entt::entity entity)
    // {
    //
    //     const auto& npcDiag = registry->get<DialogComponent>(clickedNPC);
    //     std::cout << npcDiag.sentence << std::endl;
    //     registry->get<Animation>(clickedNPC).ChangeAnimation(1); // TODO: Change to an enum
    //
    //     // Rotate to look at NPC
    //     auto selectedActor = gameData->controllableActorSystem->GetSelectedActor();
    //     auto& actorTrans = registry->get<sgTransform>(selectedActor);
    //     auto& npcTrans = registry->get<sgTransform>(clickedNPC);
    //     Vector3 direction = Vector3Subtract(npcTrans.GetWorldPos(), actorTrans.GetWorldPos());
    //     direction = Vector3Normalize(direction);
    //     float angle = atan2f(direction.x, direction.z);
    //     actorTrans.SetRotation({actorTrans.GetWorldRot().x, RAD2DEG * angle, actorTrans.GetWorldRot().z});
    //
    //     {
    //         entt::sink sink{gameData->cursor->onAnyLeftClick};
    //         sink.connect<&DialogSystem::endConversation>(this);
    //     }
    //
    //     oldCamPos = gameData->camera->GetPosition();
    //     oldCamTarget = gameData->camera->getRaylibCam()->target;
    //     gameData->camera->CutscenePose(npcTrans);
    //     gameData->camera->LockInput();
    //
    //     gameData->cursor->DisableContextSwitching();
    //     // gameData->controllableActorSystem->Disable();
    //     // TODO: Switch to dialogue state. Maybe trigger an onDialogueStart event?
    //     active = true;
    //
    //     {
    //         auto& moveableActor = registry->get<MoveableActor>(selectedActor);
    //         entt::sink sink{moveableActor.onFinishMovement};
    //         sink.disconnect<&DialogSystem::startConversation>(this);
    //         entt::sink sink2{moveableActor.onMovementCancel};
    //         sink2.disconnect<&DialogSystem::cancelConversation>(this);
    //     }
    // }
    //
    // void DialogSystem::endConversation(entt::entity actor)
    // {
    //     {
    //         entt::sink sink{gameData->cursor->onAnyLeftClick};
    //         sink.disconnect<&DialogSystem::endConversation>(this);
    //     }
    //
    //     gameData->camera->UnlockInput();
    //     gameData->cursor->EnableContextSwitching();
    //     // gameData->controllableActorSystem->Enable();
    //     // TODO: Trigger an onDialogueEnd event which changes player state.
    //     active = false;
    //     registry->get<Animation>(clickedNPC).ChangeAnimationByEnum(AnimationEnum::TALK);
    //     clickedNPC = entt::null;
    //
    //     gameData->camera->SetCamera(oldCamPos, oldCamTarget);
    //     oldCamPos = {};
    //     oldCamTarget = {};
    // }
    //
    // // TODO: This can/should be part of player state (in dialog etc)
    //
    // void DialogSystem::cancelConversation(entt::entity entity)
    // {
    //     auto& moveableActor = registry->get<MoveableActor>(selectedActor);
    //     entt::sink sink{moveableActor.onFinishMovement};
    //     sink.disconnect<&DialogSystem::startConversation>(this);
    //     entt::sink sink2{moveableActor.onMovementCancel};
    //     sink2.disconnect<&DialogSystem::cancelConversation>(this);
    //     clickedNPC = entt::null;
    // }
    //
    // void DialogSystem::NPCClicked(entt::entity _clickedNPC)
    // {
    //     if (clickedNPC != entt::null) return;
    //     clickedNPC = _clickedNPC;
    //     const auto& npc = registry->get<DialogComponent>(_clickedNPC);
    //     gameData->controllableActorSystem->PathfindToLocation(selectedActor, npc.conversationPos);
    //     {
    //         auto& moveableActor = registry->get<MoveableActor>(selectedActor);
    //         entt::sink sink{moveableActor.onFinishMovement};
    //         sink.connect<&DialogSystem::startConversation>(this);
    //         entt::sink sink2{moveableActor.onMovementCancel};
    //         sink2.connect<&DialogSystem::cancelConversation>(this);
    //     }
    // }

    void DialogSystem::StartConversation(const sgTransform& cutscenePose)
    {
        oldCamPos = gameData->camera->GetPosition();
        oldCamTarget = gameData->camera->getRaylibCam()->target;
        gameData->camera->CutscenePose(cutscenePose);
        gameData->camera->LockInput();
        gameData->cursor->DisableContextSwitching();
    }

    void DialogSystem::EndConversation()
    {
        gameData->camera->UnlockInput();
        gameData->cursor->EnableContextSwitching();
        gameData->camera->SetCamera(oldCamPos, oldCamTarget);
        oldCamPos = {};
        oldCamTarget = {};
    }

    DialogSystem::DialogSystem(entt::registry* registry, GameData* _gameData)
        : BaseSystem(registry), gameData(_gameData)
    {
        // {
        //     entt::sink sink{gameData->controllableActorSystem->onSelectedActorChange};
        //     sink.connect<&DialogSystem::changeControlledActor>(this);
        //     selectedActor = gameData->controllableActorSystem->GetSelectedActor();
        // }
        // {
        //     entt::sink sink{gameData->cursor->onNPCClick};
        //     sink.connect<&DialogSystem::NPCClicked>(this);
        // }
    }
} // namespace sage
