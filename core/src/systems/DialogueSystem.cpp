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

void DialogueSystem::startConversation(entt::entity actor)
{
    const auto& npcDiag = registry->get<Dialogue>(clickedNPC);
    onConversationStart.publish();
    std::cout << npcDiag.sentence << std::endl;
    registry->get<Animation>(clickedNPC).ChangeAnimation(1); // TODO: Change to an enum
    auto& actorTrans = registry->get<Transform>(controlledActor);
    auto& npcTrans = registry->get<Transform>(clickedNPC);

    // Rotate to look at NPC
    Vector3 direction = Vector3Subtract(npcTrans.position, actorTrans.position);
    direction = Vector3Normalize(direction);
    float angle = atan2f(direction.x, direction.z);
    actorTrans.rotation.y = RAD2DEG * angle;

    camera->CutscenePose(actorTrans);
    camera->LockInput();


    {
        entt::sink sink {actorTrans.onFinishMovement};
        sink.disconnect<&DialogueSystem::startConversation>(this);
    }
    clickedNPC = {};
}

void DialogueSystem::onNPCClicked(entt::entity _clickedNPC)
{
    clickedNPC = _clickedNPC;
    const auto& npc = registry->get<Dialogue>(_clickedNPC);
    const auto& actorCol = registry->get<Collideable>(controlledActor);
    const auto& npcCol = registry->get<Collideable>(_clickedNPC);
    actorMovementSystem->PathfindToLocation(controlledActor, npc.conversationPos);
    auto& actorTrans = registry->get<Transform>(controlledActor);
    {
        entt::sink sink {actorTrans.onFinishMovement};
        sink.connect<&DialogueSystem::startConversation>(this);
    }
}

DialogueSystem::DialogueSystem(entt::registry *registry, Cursor* _cursor, Camera* _camera, ActorMovementSystem* _actorMovementSystem) :
BaseSystem(registry), camera(_camera), actorMovementSystem(_actorMovementSystem)
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