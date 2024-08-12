#include "AbilityDefinitions.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"

#include "utils/AbilityFunctions.hpp"

#include "raylib.h"

namespace sage
{

    static constexpr AbilityData playerAutoAbilityData{
        .element = AttackElement::PHYSICAL,
        .cooldownDuration = 1,
        .baseDamage = 10,
        .range = 5,
        .animationDelay = 0,
        .repeatable = true,
    };

    void PlayerAutoAttack::Execute(entt::entity self)
    {
        auto target = registry->get<CombatableActor>(self).target;
        HitSingleTarget(registry, self, abilityData, target);
        ChangeState(self, AbilityStateEnum::IDLE);
    }

    PlayerAutoAttack::PlayerAutoAttack(entt::registry* _registry)
        : Ability(_registry, playerAutoAbilityData)
    {
    }

    static constexpr AbilityData rainoffireAbilityData{
        .cooldownDuration = 3,
        .range = 5,
        .baseDamage = 25,
        .element = AttackElement::FIRE,
        .animationDelay = 0.75f,
        .repeatable = false,
        .animationParams = {
            .animEnum = AnimationEnum::SPIN, .animSpeed = 1, .oneShot = true}};

    RainOfFire::RainOfFire(
        entt::registry* _registry,
        Camera* _camera,
        Cursor* _cursor,
        NavigationGridSystem* _navigationGridSystem)
        : CursorAbility(
              _registry,
              _camera,
              _cursor,
              std::make_unique<TextureTerrainOverlay>(
                  _registry,
                  _navigationGridSystem,
                  "resources/textures/cursor/rainoffire_cursor.png",
                  Color{255, 215, 0, 255},
                  "resources/shaders/glsl330/bloom.fs"),
              rainoffireAbilityData)
    {
    }

    static constexpr AbilityData wavemobAutoAbilityData{
        .cooldownDuration = 1,
        .range = 5,
        .baseDamage = 10,
        .element = AttackElement::PHYSICAL,
        .animationDelay = 0,
        .repeatable = true};

    void WavemobAutoAttack::Execute(entt::entity self)
    {
        auto target = registry->get<CombatableActor>(self).target;
        HitSingleTarget(registry, self, abilityData, target);
        ChangeState(self, AbilityStateEnum::IDLE);
    }

    WavemobAutoAttack::WavemobAutoAttack(entt::registry* _registry)
        : Ability(_registry, wavemobAutoAbilityData)
    {
    }

    static constexpr AbilityData whirlwindAbilityData{
        .element = AttackElement::PHYSICAL,
        .cooldownDuration = 3,
        .baseDamage = 25,
        .range = 5,
        .animationDelay = 0.65f,
        .repeatable = false,
        .animationParams = {
            .animEnum = AnimationEnum::SPIN, .animSpeed = 1, .oneShot = true}};

    void WhirlwindAbility::Execute(entt::entity self)
    {
        auto& actorTransform = registry->get<sgTransform>(self);
        Hit360AroundPoint(
            registry, self, abilityData, actorTransform.position(), whirlwindRadius);
        ChangeState(self, AbilityStateEnum::IDLE);
    }

    WhirlwindAbility::WhirlwindAbility(entt::registry* _registry)
        : Ability(_registry, whirlwindAbilityData)
    {
    }

} // namespace sage