#pragma once

#include "AbilityStateMachine.hpp"

#include <entt/entt.hpp>

namespace sage
{
    class Camera; // TODO: Why?
    class GameData;

    entt::entity CreatePlayerAutoAttack(entt::registry* registry, entt::entity caster, GameData* gameData);
    entt::entity CreateRainOfFireAbility(entt::registry* registry, entt::entity caster, GameData* gameData);
    entt::entity CreateFloorFireAbility(entt::registry* registry, entt::entity caster, GameData* gameData);
    entt::entity CreateFireballAbility(entt::registry* registry, entt::entity caster, GameData* gameData);
    entt::entity CreateLightningBallAbility(entt::registry* registry, entt::entity caster, GameData* gameData);
    entt::entity CreateWavemobAutoAttackAbility(entt::registry* registry, entt::entity caster, GameData* gameData);
    entt::entity CreateWhirlwindAbility(entt::registry* registry, entt::entity caster, GameData* gameData);

    // class RainOfFire : public AbilityStateMachine
    // {
    //     entt::entity initAbilityData(entt::registry* registry, Cursor* cursor);

    //   public:
    //     RainOfFire(entt::registry* _registry, entt::entity _self, GameData* _gameData);
    // };

    // class Fireball : public AbilityStateMachine
    // {
    //     entt::entity initAbilityData(entt::registry* registry);

    //   public:
    //     Fireball(entt::registry* _registry, entt::entity _self, GameData* _gameData);
    // };

    // class LightningBall : public AbilityStateMachine
    // {
    //     entt::entity initAbilityData(entt::registry* registry);

    //   public:
    //     LightningBall(entt::registry* _registry, entt::entity _self, GameData* _gameData);
    // };

    // class FloorFire : public AbilityStateMachine
    // {
    //     entt::entity initAbilityData(entt::registry* registry, Cursor* cursor);

    //   public:
    //     FloorFire(entt::registry* _registry, entt::entity _self, GameData* _gameData);
    // };

    // class WavemobAutoAttack : public AbilityStateMachine
    // {
    //     entt::entity initAbilityData(entt::registry* registry);

    //   public:
    //     ~WavemobAutoAttack() override = default;
    //     WavemobAutoAttack(entt::registry* _registry, entt::entity _self, GameData* _gameData);
    // };

    // class WhirlwindAbility : public AbilityStateMachine
    // {
    //     entt::entity initAbilityData(entt::registry* registry);

    //   public:
    //     float whirlwindRadius = 15.0f;
    //     ~WhirlwindAbility() override = default;
    //     WhirlwindAbility(entt::registry* _registry, entt::entity _self, GameData* _gameData);
    // };
} // namespace sage