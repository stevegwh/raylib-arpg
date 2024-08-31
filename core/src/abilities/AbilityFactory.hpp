#pragma once

#include <entt/entt.hpp>

namespace sage
{
    class GameData;

    entt::entity CreatePlayerAutoAttack(entt::registry* registry, entt::entity caster, GameData* gameData);
    entt::entity CreateRainOfFireAbility(entt::registry* registry, entt::entity caster, GameData* gameData);
    entt::entity CreateFloorFireAbility(entt::registry* registry, entt::entity caster, GameData* gameData);
    entt::entity CreateFireballAbility(entt::registry* registry, entt::entity caster, GameData* gameData);
    entt::entity CreateLightningBallAbility(entt::registry* registry, entt::entity caster, GameData* gameData);
    entt::entity CreateWavemobAutoAttackAbility(entt::registry* registry, entt::entity caster, GameData* gameData);
    entt::entity CreateWhirlwindAbility(entt::registry* registry, entt::entity caster, GameData* gameData);
} // namespace sage