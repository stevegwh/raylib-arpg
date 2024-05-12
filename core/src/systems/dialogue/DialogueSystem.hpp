//
// Created by steve on 11/05/2024.
//

#pragma once

#include "systems/BaseSystem.hpp"
#include "Cursor.hpp"
#include "Camera.hpp"
#include "components/Dialogue.hpp"
#include "DialogueWindow.hpp"
#include "systems/ActorMovementSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{

class DialogueSystem : public BaseSystem<Dialogue>
{
    bool active = false;
    entt::entity controlledActor;
    entt::entity clickedNPC;
    
    ActorMovementSystem* actorMovementSystem;
    Cursor* cursor;
    Camera* camera;
    std::unique_ptr<DialogueWindow> window;
    
    void onNPCClicked(entt::entity clickedNPC);
    void onControlledActorChange(entt::entity entity);
    void stopConversation(entt::entity entity);
    void startConversation(entt::entity actor);
public:
    explicit DialogueSystem(entt::registry* registry, 
                            Cursor* _cursor, 
                            Camera* camera, 
                            Settings* _settings,
                            ActorMovementSystem* _actorMovementSystem);
    entt::sigh<void()> onConversationStart;
    entt::sigh<void()> onConversationEnd;
    
    void Update();
    void Draw2D();
};

} // sage

