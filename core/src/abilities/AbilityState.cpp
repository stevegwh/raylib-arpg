#include "AbilityState.hpp"

namespace sage
{
    void AbilityState::Update()
    {
    }

    void AbilityState::Draw3D()
    {
    }

    void AbilityState::OnEnter()
    {
    }

    void AbilityState::OnExit()
    {
    }

    AbilityState::~AbilityState()
    {
    }

    AbilityState::AbilityState(
        entt::registry* _registry,
        entt::entity _caster,
        entt::entity _abilityEntity,
        GameData* _gameData,
        Timer& cooldownTimer,
        Timer& executionDelayTimer)
        : registry(_registry),
          caster(_caster),
          abilityEntity(_abilityEntity),
          gameData(_gameData),
          cooldownTimer(cooldownTimer),
          executionDelayTimer(executionDelayTimer)
    {
    }
}; // namespace sage