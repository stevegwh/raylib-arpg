#pragma once

#include "AbilityStateMachine.hpp"

#include <entt/entt.hpp>

namespace sage
{
    class Camera;
    class GameData;

    void CreatePlayerAutoAttack(entt::registry* registry, entt::entity caster, GameData* gameData);

    class PlayerAutoAttack : public AbilityStateMachine
    {
        entt::entity initAbilityData(entt::registry* registry);

      public:
        ~PlayerAutoAttack() override = default;
        PlayerAutoAttack(entt::registry* _registry, entt::entity _self, GameData* _gameData);
    };

    class RainOfFire : public AbilityStateMachine
    {
        entt::entity initAbilityData(entt::registry* registry, Cursor* cursor);

      public:
        RainOfFire(entt::registry* _registry, entt::entity _self, GameData* _gameData);
    };

    class Fireball : public AbilityStateMachine
    {
        entt::entity initAbilityData(entt::registry* registry);

      public:
        Fireball(entt::registry* _registry, entt::entity _self, GameData* _gameData);
    };

    class LightningBall : public AbilityStateMachine
    {
        entt::entity initAbilityData(entt::registry* registry);

      public:
        LightningBall(entt::registry* _registry, entt::entity _self, GameData* _gameData);
    };

    class FloorFire : public AbilityStateMachine
    {
        entt::entity initAbilityData(entt::registry* registry, Cursor* cursor);

      public:
        FloorFire(entt::registry* _registry, entt::entity _self, GameData* _gameData);
    };

    class WavemobAutoAttack : public AbilityStateMachine
    {
        entt::entity initAbilityData(entt::registry* registry);

      public:
        ~WavemobAutoAttack() override = default;
        WavemobAutoAttack(entt::registry* _registry, entt::entity _self, GameData* _gameData);
    };

    class WhirlwindAbility : public AbilityStateMachine
    {
        entt::entity initAbilityData(entt::registry* registry);

      public:
        float whirlwindRadius = 15.0f;
        ~WhirlwindAbility() override = default;
        WhirlwindAbility(entt::registry* _registry, entt::entity _self, GameData* _gameData);
    };
} // namespace sage