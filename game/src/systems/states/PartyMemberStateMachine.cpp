#include "PartyMemberStateMachine.hpp"
#include "animation/RpgAnimationIds.hpp"

#include "StateMachines.hpp"
#include "Systems.hpp"
#include "components/PartyMemberComponent.hpp"
#include "systems/ControllableActorSystem.hpp"

#include "engine/components/Animation.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/Cursor.hpp"
#include "engine/systems/ActorMovementSystem.hpp"

#include "raylib.h"

#include <cassert>

namespace lq
{
    void PartyMemberStateMachine::onFollowingTargetPathChanged(
        const entt::entity entity, const entt::entity target)
    {
        static constexpr int FOLLOW_DISTANCE = 15;
        const auto& trans = registry->get<sage::sgTransform>(entity);
        const auto& targetTrans = registry->get<sage::sgTransform>(target);
        const auto& targetMoveable = registry->get<sage::MoveableActor>(target);
        auto dest = targetMoveable.IsMoving() ? targetMoveable.GetDestination() : targetTrans.GetWorldPos();
        const auto dir = Vector3Normalize(Vector3Subtract(dest, trans.GetWorldPos()));
        dest = Vector3Subtract(dest, sage::Vector3MultiplyByValue(dir, FOLLOW_DISTANCE));
        sys->engine.actorMovementSystem->PathfindToLocation(entity, dest, true);
    }

    // ====== Cross-state handlers ====================================================

    void PartyMemberStateMachine::onLeaderMove(const entt::entity entity)
    {
        sys->engine.actorMovementSystem->CancelMovement(entity);
        ChangeState(entity, PartyMemberFollowingLeaderState{});
    }

    // ====== Lifecycle ===============================================================

    void PartyMemberStateMachine::Update()
    {
        for (const auto view = registry->view<PartyMemberState>(); const auto& entity : view)
        {
            assert(!registry->any_of<PlayerState>(entity));
            auto& state = registry->get<PartyMemberState>(entity);
            std::visit([this, entity](auto& cur) { cur.Update(*this, entity); }, state.current);
        }
    }

    void PartyMemberStateMachine::Draw3D()
    {
    }

    void PartyMemberStateMachine::onComponentAdded(const entt::entity entity)
    {
        auto& partyMember = registry->get<PartyMemberComponent>(entity);
        partyMember.followTarget = sys->selectionSystem->GetSelectedActor();
        auto& target = registry->get<sage::MoveableActor>(partyMember.followTarget.value());
        target.onStartMovement.Subscribe([this, entity](entt::entity) { onLeaderMove(entity); });

        auto& state = registry->get<PartyMemberState>(entity);
        std::visit([this, entity](auto& cur) { cur.OnEnter(*this, entity); }, state.current);
    }

    PartyMemberStateMachine::PartyMemberStateMachine(entt::registry* _registry, Systems* _sys)
        : Base(_registry), sys(_sys)
    {
        registry->on_construct<PartyMemberState>().connect<&PartyMemberStateMachine::onComponentAdded>(this);
        registry->on_destroy<PartyMemberState>().connect<&PartyMemberStateMachine::onComponentRemoved>(this);
    }
} // namespace lq
