#pragma once

#include "AbilityData.hpp"
#include "Timer.hpp"

#include <entt/entt.hpp>
#include <memory>
#include <unordered_map>

namespace sage
{
    class GameData;
    class VisualFX;
    class AbilityState;

    enum class AbilityStateEnum
    {
        IDLE,
        CURSOR_SELECT,
        AWAITING_EXECUTION
    };

    class AbilityStateMachine
    {
        class IdleState;
        class AwaitingExecutionState;
        class CursorSelectState;

        GameData* gameData;

        template <typename AbilityFunc>
        AbilityFunc& GetExecuteFunc(
            entt::registry* _registry, entt::entity caster, entt::entity _abilityDataEntity, GameData* _gameData)
        {
            if (_registry->any_of<AbilityFunc>(_abilityDataEntity))
            {
                return _registry->get<AbilityFunc>(_abilityDataEntity);
            }
            else
            {
                return _registry->emplace<AbilityFunc>(
                    _abilityDataEntity, _registry, caster, _abilityDataEntity, _gameData);
            }
        }

      protected:
        entt::registry* registry;
        entt::entity caster;
        entt::entity abilityEntity;
        // AbilityData abilityData;
        Timer cooldownTimer;
        Timer executionDelayTimer;

        std::unique_ptr<VisualFX> vfx;
        // std::unique_ptr<AbilityFunction> executeFunc;

        AbilityState* state;
        std::unordered_map<AbilityStateEnum, std::unique_ptr<AbilityState>> states;
        void ChangeState(AbilityStateEnum newState);

      public:
        virtual void ResetCooldown();
        virtual bool IsActive();
        float GetRemainingCooldownTime() const;
        float GetCooldownDuration() const;
        bool CooldownReady() const;

        virtual void Cancel();
        void Execute();

        virtual void Update();
        virtual void Draw3D();
        virtual void Init();
        void Confirm();

        virtual ~AbilityStateMachine();
        AbilityStateMachine(const AbilityStateMachine&) = delete;
        AbilityStateMachine& operator=(const AbilityStateMachine&) = delete;
        AbilityStateMachine(
            entt::registry* _registry, entt::entity _caster, entt::entity _abilityEntity, GameData* _gameData);
    };

} // namespace sage