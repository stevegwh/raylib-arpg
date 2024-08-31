#pragma once

#include "abilities/AbilityData.hpp"
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

    class AbilitySystem
    {
        class IdleState;
        class AwaitingExecutionState;
        class CursorSelectState;

        entt::registry* registry;
        GameData* gameData;
        std::unordered_map<AbilityStateEnum, std::unique_ptr<AbilityState>> states;

        template <typename AbilityFunc>
        AbilityFunc& getExecuteFunc(
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

        void changeState(entt::entity abilityEntity, AbilityStateEnum newState);
        void executeAbility(entt::entity abilityEntity);
        void confirmAbility(entt::entity abilityEntity);

      public:
        void CancelAbility(entt::entity abilityEntity);
        void InitAbility(entt::entity abilityEntity);
        void Update();
        void Draw3D();

        ~AbilitySystem();
        AbilitySystem(const AbilitySystem&) = delete;
        AbilitySystem& operator=(const AbilitySystem&) = delete;
        AbilitySystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage