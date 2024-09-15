//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ControllableActorSystem.hpp"
#include "GameData.hpp"

#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "systems/ActorMovementSystem.hpp"

namespace sage
{
    void ControllableActorSystem::Update() const
    {
        auto view = registry->view<ControllableActor>();
        for (auto& entity : view)
        {
            auto& controlledActor = registry->get<ControllableActor>(entity);
            if (controlledActor.targetActor != entt::null)
            {
                controlledActor.checkTargetPosTimer.Update(GetFrameTime());
            }
        }
    }

    bool ControllableActorSystem::ReachedDestination(entt::entity entity) const
    {
        return gameData->actorMovementSystem->ReachedDestination(entity);
    }

    void ControllableActorSystem::onTargetUpdate(entt::entity target)
    {
        auto& controlledActor = registry->get<ControllableActor>(controlledActorId);
        if (controlledActor.checkTargetPosTimer.HasFinished())
        {
            controlledActor.checkTargetPosTimer.Restart();
            auto& targetTrans = registry->get<sgTransform>(target);
            PathfindToLocation(controlledActorId, targetTrans.GetWorldPos());
        }
    }

    void ControllableActorSystem::CancelMovement(entt::entity entity)
    {
        auto& controlledActor = registry->get<ControllableActor>(entity);
        if (controlledActor.targetActor != entt::null && registry->valid(controlledActor.targetActor))
        {
            auto& target = registry->get<sgTransform>(controlledActor.targetActor);
            {
                entt::sink sink{target.onPositionUpdate};
                sink.disconnect<&ControllableActorSystem::onTargetUpdate>(this);
            }
        }

        gameData->actorMovementSystem->CancelMovement(entity);
    }

    void ControllableActorSystem::PathfindToLocation(entt::entity id, Vector3 location)
    {
        gameData->actorMovementSystem->PathfindToLocation(id, location);
    }

    void ControllableActorSystem::MoveToLocation(entt::entity id)
    {
        gameData->actorMovementSystem->PathfindToLocation(id, {gameData->cursor->collision().point});
    }

    void ControllableActorSystem::PatrolLocations(entt::entity id, const std::vector<Vector3>& patrol)
    {
        // actorMovementSystem->PathfindToLocation(id, patrol);
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

    ControllableActorSystem::ControllableActorSystem(entt::registry* _registry, GameData* _gameData)
        : BaseSystem(_registry), gameData(_gameData)

    {

        entt::sink sink{onControlledActorChange};
        sink.connect<&Cursor::OnControlledActorChange>(gameData->cursor.get());
    }
} // namespace sage
