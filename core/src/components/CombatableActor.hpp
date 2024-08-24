//
// Created by Steve Wheeler on 04/06/2024.
//

#pragma once

#include <entt/entt.hpp>
#include <memory>

#include "EntityEventBridge.hpp"

namespace sage
{
    enum class CombatableActorType
    {
        PLAYER,
        WAVEMOB
    };

    enum class AttackElement
    {
        PHYSICAL,
        FIRE,
        ICE,
        LIGHTNING,
        POISON
    };

    struct AttackData
    {
        const entt::entity attacker;
        const entt::entity hit;
        const int damage;
        const AttackElement element = AttackElement::PHYSICAL;
    };

    struct CombatData
    {
        int hp;
        int maxHp;
    };

    class CombatableActor
    {

      public:
        int hp = 100;
        CombatableActorType actorType = CombatableActorType::WAVEMOB;
        bool dying = false;
        entt::entity target{};
        int attackRange = 5;

        entt::sigh<void(AttackData)> onHit{}; // Self, attacker, damage
        entt::sigh<void(entt::entity)> onDeath{};
        entt::sigh<void(entt::entity, entt::entity)> onAttackCancelled{}; // Self, object clicked (can discard)
        entt::sigh<void(entt::entity, entt::entity)> onTargetDeath{};     // Self, target (that died)
    };
} // namespace sage
