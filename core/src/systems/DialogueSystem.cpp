//
// Created by steve on 11/05/2024.
//

#include "DialogueSystem.hpp"
#include "components/Collideable.hpp"
#include "components/Animation.hpp"

#include <iostream>

namespace sage
{
void DialogueSystem::onControlledActorChange(entt::entity entity)
{
    controlledActor = entity;
}

void DialogueSystem::startConversation(entt::entity entity)
{
    onConversationStart.publish();

    const auto& npcDiag = registry->get<Dialogue>(clickedNPC);
    std::cout << npcDiag.sentence << std::endl;
    registry->get<Animation>(clickedNPC).ChangeAnimation(1); // TODO: Change to an enum

    // Rotate to look at NPC
    auto& actorTrans = registry->get<Transform>(controlledActor);
    auto& npcTrans = registry->get<Transform>(clickedNPC);
    Vector3 direction = Vector3Subtract(npcTrans.position, actorTrans.position);
    direction = Vector3Normalize(direction);
    float angle = atan2f(direction.x, direction.z);
    actorTrans.rotation.y = RAD2DEG * angle;

    camera->CutscenePose(npcTrans);
    camera->LockInput();
    cursor->LockContext();

    stopConversation(entity);
}

void DialogueSystem::stopConversation(entt::entity entity)
{
    {
        auto& actorTrans = registry->get<Transform>(entity);
        entt::sink sink {actorTrans.onFinishMovement};
        sink.disconnect<&DialogueSystem::startConversation>(this);
        entt::sink sink2 {actorTrans.onMovementCancel};
        sink2.disconnect<&DialogueSystem::stopConversation>(this);
        clickedNPC = entt::null;
    }
}

void DialogueSystem::onNPCClicked(entt::entity _clickedNPC)
{
    if (clickedNPC != entt::null) return;
    clickedNPC = _clickedNPC;
    const auto& npc = registry->get<Dialogue>(_clickedNPC);
    const auto& actorCol = registry->get<Collideable>(controlledActor);
    const auto& npcCol = registry->get<Collideable>(_clickedNPC);
    actorMovementSystem->PathfindToLocation(controlledActor, npc.conversationPos);
    auto& actorTrans = registry->get<Transform>(controlledActor);
    {
        entt::sink sink {actorTrans.onFinishMovement};
        sink.connect<&DialogueSystem::startConversation>(this);
        entt::sink sink2 {actorTrans.onMovementCancel};
        sink2.connect<&DialogueSystem::stopConversation>(this);
    }
}

DialogueSystem::DialogueSystem(entt::registry *registry, 
                               Cursor* _cursor, 
                               Camera* _camera, 
                               ActorMovementSystem* _actorMovementSystem) :
    BaseSystem(registry), 
    cursor(_cursor), 
    camera(_camera), 
    actorMovementSystem(_actorMovementSystem), 
    clickedNPC(entt::null)
{
    {
        entt::sink sink{_actorMovementSystem->onControlledActorChange};
        sink.connect<&DialogueSystem::onControlledActorChange>(this);
        controlledActor = _actorMovementSystem->GetControlledActor();
    }
    {
        entt::sink sink{_cursor->onNPCClick};
        sink.connect<&DialogueSystem::onNPCClicked>(this);
    }
}
} // sage