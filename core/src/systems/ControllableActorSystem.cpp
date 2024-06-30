//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ControllableActorSystem.hpp"
#include "../Application.hpp"

namespace sage
{

void ControllableActorSystem::Update() const
{
    auto view = registry->view<ControllableActor>();
    for (auto& entity : view)
    {
        auto& actor = registry->get<ControllableActor>(entity);
        if (actor.targetActor != entt::null)
        {
	        actor.checkTargetPosTimer += GetFrameTime();
        }
    }
}

void ControllableActorSystem::onTargetUpdate(entt::entity target)
{
    auto& actor = registry->get<ControllableActor>(controlledActorId);
    if (actor.checkTargetPosTimer > actor.checkTargetPosThreshold)
    {
        actor.checkTargetPosTimer = 0;
        auto& targetTrans = registry->get<Transform>(target);
        PathfindToLocation(controlledActorId, targetTrans.position);
    }
}

void ControllableActorSystem::CancelMovement(entt::entity entity)
{
    if (!registry->any_of<ControllableActor>(entity)) return;
    auto& actor = registry->get<ControllableActor>(entity);
    if (actor.targetActor != entt::null) // TODO: Unsure if this properly unsubscribes
    {
	    auto& target = registry->get<Transform>(actor.targetActor);
	    {
	        entt::sink sink { target.onPositionUpdate };
	        sink.disconnect<&ControllableActorSystem::onTargetUpdate>(this);
	    }
	    {
	        entt::sink sink { target.onMovementCancel };
	        sink.disconnect<&ControllableActorSystem::CancelMovement>(this);
	    }
	    {
	        entt::sink sink { target.onFinishMovement };
	        sink.disconnect<&ControllableActorSystem::CancelMovement>(this);
	    }
    }

    actorMovementSystem->CancelMovement(entity);
}

void ControllableActorSystem::PathfindToLocation(entt::entity id, Vector3 location)
{
    actorMovementSystem->PathfindToLocation(id, location);
}

void ControllableActorSystem::MoveToLocation(entt::entity id)
{
    actorMovementSystem->PathfindToLocation(id, { cursor->collision.point });
}

void ControllableActorSystem::PatrolLocations(entt::entity id, const std::vector<Vector3>& patrol)
{
    //actorMovementSystem->PathfindToLocation(id, patrol);
}

void ControllableActorSystem::onFloorClick(entt::entity entity)
{
    actorMovementSystem->CancelMovement(controlledActorId); // Flush any previous commands
    PathfindToLocation(controlledActorId, cursor->collision.point);
}

void ControllableActorSystem::onEnemyClick(entt::entity entity)
{
    auto& controlledActor = registry->get<ControllableActor>(controlledActorId);
    actorMovementSystem->CancelMovement(controlledActorId); // Flush any previous commands
    auto& target = registry->get<Transform>(entity);
    controlledActor.targetActor = entity;
    controlledActor.targetActorPos = target.position;
    {
        entt::sink sink { target.onPositionUpdate };
        sink.connect<&ControllableActorSystem::onTargetUpdate>(this);
    }
    {
        entt::sink sink { target.onMovementCancel };
        sink.connect<&ControllableActorSystem::CancelMovement>(this);
    }
    {
        entt::sink sink { target.onFinishMovement };
        sink.connect<&ControllableActorSystem::CancelMovement>(this);
    }

    PathfindToLocation(controlledActorId, cursor->collision.point);
}
    
void ControllableActorSystem::SetControlledActor(entt::entity id)
{
    onControlledActorChange.publish(id);
    controlledActorId = id;
}

entt::entity ControllableActorSystem::GetControlledActor()
{
    return controlledActorId;
}

void ControllableActorSystem::Enable()
{
    {
        entt::sink onClick{ cursor->onFloorClick };
        onClick.connect<&ControllableActorSystem::onFloorClick>(this);
    }

    {
        entt::sink onClick{ cursor->onEnemyClick };
        onClick.connect<&ControllableActorSystem::onEnemyClick>(this);
    }
}

void ControllableActorSystem::Disable()
{
    {
        entt::sink onClick{ cursor->onFloorClick };
        onClick.disconnect<&ControllableActorSystem::onFloorClick>(this);
    }

    {
        entt::sink onClick{ cursor->onEnemyClick };
        onClick.disconnect<&ControllableActorSystem::onEnemyClick>(this);
    }
}

ControllableActorSystem::ControllableActorSystem(entt::registry* _registry,
                                                                 Cursor* _cursor,
                                                                 UserInput* _userInput,
                                                                 NavigationGridSystem* _navigationGridSystem,
                                                                 ActorMovementSystem* _transformSystem) :
    BaseSystem<ControllableActor>(_registry), cursor(_cursor), userInput(_userInput),
    navigationGridSystem(_navigationGridSystem), actorMovementSystem(_transformSystem)
{
    Enable();
}

} // sage
