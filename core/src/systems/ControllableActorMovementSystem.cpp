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
        if (actor.checkTargetPosTimer > actor.checkTargetPosThreshold)
        {
            actor.checkTargetPosTimer = 0;
            auto& targetCurrentPos = registry->get<Transform>(actor.target);
            if (actor.targetPos.x != targetCurrentPos.position.x && actor.targetPos.z != targetCurrentPos.position.z)
            {
                PathfindToLocation(entity, targetCurrentPos.position);
            }
        }
        else
        {
            actor.checkTargetPosTimer += GetFrameTime();
        }
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
    transformSystem->PathfindToLocation(id, {cursor->collision.point});
}

void ControllableActorMovementSystem::PatrolLocations(entt::entity id, const std::vector<Vector3>& patrol)
{
    transformSystem->PathfindToLocation(id, patrol);
}

void ControllableActorMovementSystem::onCursorClick()
{
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
    entt::sink onClick{cursor->onFloorClick};
    onClick.connect<&ControllableActorMovementSystem::onCursorClick>(this);
}

void ControllableActorMovementSystem::Disable()
{
    entt::sink onClick{cursor->onFloorClick};
    onClick.disconnect<&ControllableActorMovementSystem::onCursorClick>(this);
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
