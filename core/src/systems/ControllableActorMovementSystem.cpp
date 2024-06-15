//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ControllableActorMovementSystem.hpp"
#include "../Application.hpp"

namespace sage
{

void ControllableActorMovementSystem::Update()
{
    auto view = registry->view<ControllableActor>();
    for (auto& entity : view)
    {
        auto& actor = registry->get<ControllableActor>(entity);
        actor.checkTargetPosTimer += GetFrameTime();
    }
}

void ControllableActorMovementSystem::onTargetUpdate(entt::entity target)
{
    auto& actor = registry->get<ControllableActor>(controlledActorId);
    if (actor.checkTargetPosTimer > actor.checkTargetPosThreshold)
    {
        actor.checkTargetPosTimer = 0;
        auto& targetTrans = registry->get<Transform>(target);
        PathfindToLocation(controlledActorId, targetTrans.position);
    }
}

void ControllableActorMovementSystem::cancelMovement(entt::entity entity)
{
    auto& actor = registry->get<ControllableActor>(entity);
    auto& target = registry->get<Transform>(actor.targetActor);
    {
        entt::sink sink { target.onPositionUpdate };
        sink.disconnect<&ControllableActorMovementSystem::onTargetUpdate>(this);
    }
    {
        entt::sink sink { target.onMovementCancel };
        sink.disconnect<&ControllableActorMovementSystem::cancelMovement>(this);
    }
    {
        entt::sink sink { target.onFinishMovement };
        sink.disconnect<&ControllableActorMovementSystem::cancelMovement>(this);
    }
}

void ControllableActorMovementSystem::PathfindToLocation(entt::entity id, Vector3 location)
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
    navigationGridSystem->DrawDebugPathfinding(minRange, maxRange);

    const auto& actorPos = registry->get<Transform>(id);
    auto path = navigationGridSystem->Pathfind(actorPos.position, location, minRange, maxRange);
    if (!path.empty()) transformSystem->PathfindToLocation(id, path);
}

void ControllableActorMovementSystem::MoveToLocation(entt::entity id)
{
    transformSystem->PathfindToLocation(id, { cursor->collision.point });
}

void ControllableActorMovementSystem::PatrolLocations(entt::entity id, const std::vector<Vector3>& patrol)
{
    transformSystem->PathfindToLocation(id, patrol);
}

void ControllableActorMovementSystem::onFloorClick(entt::entity entity)
{
    transformSystem->CancelMovement(controlledActorId); // Flush any previous commands
    PathfindToLocation(controlledActorId, cursor->collision.point);
}

void ControllableActorMovementSystem::onEnemyClick(entt::entity entity)
{
    auto& controlledActor = registry->get<ControllableActor>(controlledActorId);
    transformSystem->CancelMovement(controlledActorId); // Flush any previous commands
    auto& target = registry->get<Transform>(entity);
    controlledActor.targetActor = entity;
    controlledActor.targetActorPos = target.position;
    {
        entt::sink sink { target.onPositionUpdate };
        sink.connect<&ControllableActorMovementSystem::onTargetUpdate>(this);
    }
    {
        entt::sink sink { target.onMovementCancel };
        sink.connect<&ControllableActorMovementSystem::cancelMovement>(this);
    }
    {
        entt::sink sink { target.onFinishMovement };
        sink.connect<&ControllableActorMovementSystem::cancelMovement>(this);
    }
    PathfindToLocation(controlledActorId, cursor->collision.point);
}
    
void ControllableActorMovementSystem::SetControlledActor(entt::entity id)
{
    onControlledActorChange.publish(id);
    controlledActorId = id;
}

entt::entity ControllableActorMovementSystem::GetControlledActor()
{
    return controlledActorId;
}

void ControllableActorMovementSystem::Enable()
{
    {
        entt::sink onClick{ cursor->onFloorClick };
        onClick.connect<&ControllableActorMovementSystem::onFloorClick>(this);
    }

    {
        entt::sink onClick{ cursor->onEnemyClick };
        onClick.connect<&ControllableActorMovementSystem::onEnemyClick>(this);
    }
}

void ControllableActorMovementSystem::Disable()
{
    {
        entt::sink onClick{ cursor->onFloorClick };
        onClick.disconnect<&ControllableActorMovementSystem::onFloorClick>(this);
    }

    {
        entt::sink onClick{ cursor->onEnemyClick };
        onClick.disconnect<&ControllableActorMovementSystem::onEnemyClick>(this);
    }
}

ControllableActorMovementSystem::ControllableActorMovementSystem(entt::registry* _registry,
                                                                 Cursor* _cursor,
                                                                 UserInput* _userInput,
                                                                 NavigationGridSystem* _navigationGridSystem,
                                                                 TransformSystem* _transformSystem) :
    BaseSystem<ControllableActor>(_registry), cursor(_cursor), userInput(_userInput),
    navigationGridSystem(_navigationGridSystem), transformSystem(_transformSystem)
{
    Enable();
}

} // sage
