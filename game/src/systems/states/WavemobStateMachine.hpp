// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "WavemobStates.hpp"
#include "engine/systems/states/StateMachineBase.hpp"

#include "entt/entt.hpp"

#include <variant>

namespace lq
{
    class Systems;
    struct AttackData;

    class WavemobStateMachine final : public sage::StateMachineBase<WavemobStateMachine, WavemobState>
    {
        using Base = sage::StateMachineBase<WavemobStateMachine, WavemobState>;
        friend Base;
        friend struct WavemobDefaultState;
        friend struct WavemobTargetOutOfRangeState;
        friend struct WavemobCombatState;
        friend struct WavemobDyingState;

        // Dying is terminal — block all further transitions away from it.
        static bool isLocked(const WavemobState& s)
        {
            return std::holds_alternative<WavemobDyingState>(s.current);
        }

        Systems* sys;

        template <typename State>
        void onEnter(State& state, const entt::entity entity)
        {
            state.OnEnter(*this, entity);
        }

        template <typename State>
        void onExit(State& state, const entt::entity entity)
        {
            state.OnExit(*this, entity);
        }

        void onHit(AttackData attackData);
        void onDeath(entt::entity entity);
        [[nodiscard]] bool isTargetOutOfSight(entt::entity entity) const;
        void onTargetPosUpdate(entt::entity entity, entt::entity target) const;
        void destroyEntity(entt::entity entity);

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity) const
        {
        }

      public:
        void Update();
        void Draw3D();

        ~WavemobStateMachine() = default;
        WavemobStateMachine(entt::registry* _registry, Systems* _sys);
    };
} // namespace lq
