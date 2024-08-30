#pragma once

#include "AbilityData.hpp"

#include "components/Ability.hpp"
#include "Timer.hpp"

#include <entt/entt.hpp>
#include <memory>
#include <unordered_map>

namespace sage
{
    class GameData;
    class VisualFX;
    class AbilityState;

    class AbilityStateMachine
    {
        class IdleState;
        class AwaitingExecutionState;
        class CursorSelectState;

        entt::registry* registry;
        std::unordered_map<AbilityStateEnum, std::unique_ptr<AbilityState>> states;

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

        void ChangeState(entt::entity abilityEntity, AbilityStateEnum newState);

      public:
        void Cancel(entt::entity abilityEntity);
        void Execute(entt::entity abilityEntity);

        void Update();
        void Draw3D();
        void Init(entt::entity abilityEntity);
        void Confirm(entt::entity abilityEntity);

        ~AbilityStateMachine();
        AbilityStateMachine(const AbilityStateMachine&) = delete;
        AbilityStateMachine& operator=(const AbilityStateMachine&) = delete;
        AbilityStateMachine(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage