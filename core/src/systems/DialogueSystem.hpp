//
// Created by steve on 11/05/2024.
//

#pragma once

#include "BaseSystem.hpp"
#include "Cursor.hpp"
#include "Camera.hpp"
#include "../components/Dialogue.hpp"
#include "ActorMovementSystem.hpp"

#include <entt/entt.hpp>

namespace sage
{

class DialogueSystem : public BaseSystem<Dialogue>
{
    entt::entity controlledActor;
    entt::entity clickedNPC;
    ActorMovementSystem* actorMovementSystem;
    TransformSystem* transformSystem;
    Camera* camera;
    void onNPCClicked(entt::entity clickedNPC);
    void onControlledActorChange(entt::entity entity);
    void startConversation(entt::entity actor);
public:
    explicit DialogueSystem(entt::registry* registry, Cursor* _cursor, Camera* camera, ActorMovementSystem* _actorMovementSystem);
    entt::sigh<void()> onConversationStart;
    entt::sigh<void()> onConversationEnd;
};

} // sage

