//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ControllableActorSystem.hpp"
#include "../Application.hpp"

namespace sage
{

void ControllableActorSystem::Update()
{
    auto view = registry->view<ControllableActor>();
    for (auto& entity : view)
    {
        auto& actor = registry->get<ControllableActor>(entity);
        actor.checkTargetPosTimer += GetFrameTime();
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

void ControllableActorSystem::cancelMovement(entt::entity entity)
{
    auto& actor = registry->get<ControllableActor>(entity);
    auto& target = registry->get<Transform>(actor.targetActor);
    {
        entt::sink sink { target.onPositionUpdate };
        sink.disconnect<&ControllableActorSystem::onTargetUpdate>(this);
    }
    {
        entt::sink sink { target.onMovementCancel };
        sink.disconnect<&ControllableActorSystem::cancelMovement>(this);
    }
    {
        entt::sink sink { target.onFinishMovement };
        sink.disconnect<&ControllableActorSystem::cancelMovement>(this);
    }
}

void ControllableActorSystem::PathfindToLocation(entt::entity id, Vector3 location)
{
    {
        // If location outside of bounds, then return
        Vector2 tmp;
        if (!navigationGridSystem->WorldToGridSpace(location, tmp)) return;
    }
    const auto& actor = registry->get<ControllableActor>(id);
    Vector2 minRange;
    Vector2 maxRange;
    navigationGridSystem->GetPathfindRange(id, actor.pathfindingBounds, minRange, maxRange);
    {
        // If location outside of actor's movement range, then return
        Vector2 tmp;
        if (!navigationGridSystem->WorldToGridSpace(location, tmp, minRange, maxRange)) return;
    }
    navigationGridSystem->DrawDebugPathfinding(minRange, maxRange);

    const auto& actorPos = registry->get<Transform>(id);
    auto path = navigationGridSystem->AStarPathfind(id, actorPos.position, location, minRange, maxRange);
    if (!path.empty()) transformSystem->PathfindToLocation(id, path);
}

void ControllableActorSystem::MoveToLocation(entt::entity id)
{
    transformSystem->PathfindToLocation(id, { cursor->collision.point });
}

void ControllableActorSystem::PatrolLocations(entt::entity id, const std::vector<Vector3>& patrol)
{
    transformSystem->PathfindToLocation(id, patrol);
}

void ControllableActorSystem::onFloorClick(entt::entity entity)
{
    transformSystem->CancelMovement(controlledActorId); // Flush any previous commands
    PathfindToLocation(controlledActorId, cursor->collision.point);
}

void ControllableActorSystem::onEnemyClick(entt::entity entity)
{
    auto& controlledActor = registry->get<ControllableActor>(controlledActorId);
    transformSystem->CancelMovement(controlledActorId); // Flush any previous commands
    auto& target = registry->get<Transform>(entity);
    controlledActor.targetActor = entity;
    controlledActor.targetActorPos = target.position;
    {
        entt::sink sink { target.onPositionUpdate };
        sink.connect<&ControllableActorSystem::onTargetUpdate>(this);
    }
    {
        entt::sink sink { target.onMovementCancel };
        sink.connect<&ControllableActorSystem::cancelMovement>(this);
    }
    {
        entt::sink sink { target.onFinishMovement };
        sink.connect<&ControllableActorSystem::cancelMovement>(this);
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
    navigationGridSystem(_navigationGridSystem), transformSystem(_transformSystem)
{
    Enable();
}

} // sage
