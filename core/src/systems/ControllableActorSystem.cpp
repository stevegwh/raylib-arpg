//
// Created by Steve Wheeler on 29/02/2024.
//

#include "ControllableActorSystem.hpp"

#include "ActorMovementSystem.hpp"
#include "Cursor.hpp"
#include "NavigationGridSystem.hpp"
#include "UserInput.hpp"

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
        return actorMovementSystem->ReachedDestination(entity);
    }

    void ControllableActorSystem::onTargetUpdate(entt::entity target)
    {
        auto& controlledActor = registry->get<ControllableActor>(controlledActorId);
        if (controlledActor.checkTargetPosTimer.HasFinished())
        {
            controlledActor.checkTargetPosTimer.Restart();
            auto& targetTrans = registry->get<sgTransform>(target);
            PathfindToLocation(controlledActorId, targetTrans.position());
        }
    }

    void ControllableActorSystem::CancelMovement(entt::entity entity)
    {
        if (!registry->any_of<ControllableActor>(entity)) return; // TODO: Necessary?
        auto& controlledActor = registry->get<ControllableActor>(entity);
        if (controlledActor.targetActor != entt::null &&
            registry->valid(controlledActor.targetActor))
        {
            auto& target = registry->get<sgTransform>(controlledActor.targetActor);
            {
                entt::sink sink{target.onPositionUpdate};
                sink.disconnect<&ControllableActorSystem::onTargetUpdate>(this);
            }
        }
        auto& trans = registry->get<sgTransform>(entity);
        {
            entt::sink sink{trans.onMovementCancel};
            sink.disconnect<&ControllableActorSystem::CancelMovement>(this);
        }
        {
            entt::sink sink{trans.onFinishMovement};
            sink.disconnect<&ControllableActorSystem::CancelMovement>(this);
        }

        actorMovementSystem->CancelMovement(entity);
    }

    void ControllableActorSystem::onEnemyClick(entt::entity clickedEntity)
    {
        // Flush any previous commands
        actorMovementSystem->CancelMovement(controlledActorId);
        auto& controlledActor = registry->get<ControllableActor>(controlledActorId);
        controlledActor.targetActor = clickedEntity;
        auto& targetTrans = registry->get<sgTransform>(clickedEntity);
        controlledActor.targetActorPos = targetTrans.position();
        {
            entt::sink sink{targetTrans.onPositionUpdate};
            sink.connect<&ControllableActorSystem::onTargetUpdate>(this);
        }
        auto& trans = registry->get<sgTransform>(controlledActorId);
        {
            entt::sink sink{trans.onMovementCancel};
            sink.connect<&ControllableActorSystem::CancelMovement>(this);
        }
        {
            entt::sink sink{trans.onFinishMovement};
            sink.connect<&ControllableActorSystem::CancelMovement>(this);
        }

        PathfindToLocation(controlledActorId, targetTrans.position());
    }

    void ControllableActorSystem::PathfindToLocation(entt::entity id, Vector3 location)
    {
        actorMovementSystem->PathfindToLocation(id, location);
    }

    void ControllableActorSystem::MoveToLocation(entt::entity id)
    {
        actorMovementSystem->PathfindToLocation(id, {cursor->collision().point});
    }

    void ControllableActorSystem::PatrolLocations(
        entt::entity id, const std::vector<Vector3>& patrol)
    {
        // actorMovementSystem->PathfindToLocation(id, patrol);
    }

    void ControllableActorSystem::onFloorClick(entt::entity entity)
    {
        CancelMovement(controlledActorId); // Flush any previous commands
        PathfindToLocation(controlledActorId, cursor->collision().point);
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
            entt::sink onClick{cursor->onFloorClick};
            onClick.connect<&ControllableActorSystem::onFloorClick>(this);
        }
        {
            entt::sink onClick{cursor->onEnemyClick};
            onClick.connect<&ControllableActorSystem::onEnemyClick>(this);
        }
    }

    void ControllableActorSystem::Disable()
    {
        {
            entt::sink onClick{cursor->onFloorClick};
            onClick.disconnect<&ControllableActorSystem::onFloorClick>(this);
        }
        {
            entt::sink onClick{cursor->onEnemyClick};
            onClick.disconnect<&ControllableActorSystem::onEnemyClick>(this);
        }
    }

    ControllableActorSystem::ControllableActorSystem(
        entt::registry* _registry,
        Cursor* _cursor,
        UserInput* _userInput,
        NavigationGridSystem* _navigationGridSystem,
        ActorMovementSystem* _transformSystem)
        : BaseSystem(_registry),
          cursor(_cursor),
          userInput(_userInput),
          navigationGridSystem(_navigationGridSystem),
          actorMovementSystem(_transformSystem)
    {
        Enable();
        {
            entt::sink sink{onControlledActorChange};
            sink.connect<&Cursor::OnControlledActorChange>(*cursor);
        }
    }
} // namespace sage
