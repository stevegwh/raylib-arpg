//
// Created by steve on 11/05/2024.
//

#pragma once

#include "systems/BaseSystem.hpp"
#include "Cursor.hpp"
#include "Camera.hpp"
#include "components/Dialogue.hpp"
#include "DialogueWindow.hpp"
#include "systems/ControllableActorSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{

class DialogueSystem : public BaseSystem<Dialogue>
{
    bool active = false;
    entt::entity controlledActor;
    entt::entity clickedNPC;
    
    ControllableActorSystem* actorMovementSystem;
    Cursor* cursor;
    Camera* camera;
    Vector3 oldCamPos;
    Vector3 oldCamTarget;
    std::unique_ptr<DialogueWindow> window;
    
    void NPCClicked(entt::entity _clickedNPC);
    void changeControlledActor(entt::entity entity);
    void cancelConversation(entt::entity entity);
    void startConversation(entt::entity actor);
    void endConversation(entt::entity actor);
public:
    explicit DialogueSystem(entt::registry* registry,
                            Cursor* _cursor,
                            Camera* camera,
                            Settings* _settings,
                            ControllableActorSystem* _actorMovementSystem);
    entt::sigh<void()> onConversationStart;
    entt::sigh<void()> onConversationEnd;
    
    void Update();
    void Draw2D();
};

} // sage

