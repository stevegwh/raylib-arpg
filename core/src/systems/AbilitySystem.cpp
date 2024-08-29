#include "AbilitySystem.hpp"

#include "abilities/Abilities.hpp"
#include "abilities/Ability.hpp"
#include "GameData.hpp"

namespace sage
{

    Ability* AbilitySystem::GetAbility(entt::entity entity, AbilityEnum abilityEnum)
    {
        if (!abilityMap.contains(entity))
        {
            return nullptr;
        }

        if (abilityMap[entity].contains(abilityEnum))
        {
            return abilityMap[entity][abilityEnum].get();
        }

        return nullptr;
    }

    std::vector<Ability*> AbilitySystem::GetAbilities(entt::entity entity)
    {
        std::vector<Ability*> abilities;
        for (const auto& ability : abilityMap[entity])
        {
            abilities.push_back(ability.second.get());
        }
        return abilities;
    }

    Ability* AbilitySystem::RegisterAbility(entt::entity entity, AbilityEnum abilityEnum)
    {
        if (abilityEnum == AbilityEnum::PLAYER_AUTOATTACK)
        {
            auto obj = std::make_unique<PlayerAutoAttack>(registry, entity, gameData);
            abilityMap[entity].emplace(abilityEnum, std::move(obj));
        }
        else if (abilityEnum == AbilityEnum::ENEMY_AUTOATTACK)
        {
            auto obj = std::make_unique<WavemobAutoAttack>(registry, entity, gameData);
            abilityMap[entity].emplace(abilityEnum, std::move(obj));
        }
        else if (abilityEnum == AbilityEnum::FIREBALL)
        {
            auto obj = std::make_unique<Fireball>(registry, entity, gameData);
            abilityMap[entity].emplace(abilityEnum, std::move(obj));
        }
        else if (abilityEnum == AbilityEnum::LIGHTNINGBALL)
        {
            auto obj = std::make_unique<LightningBall>(registry, entity, gameData);
            abilityMap[entity].emplace(abilityEnum, std::move(obj));
        }
        else if (abilityEnum == AbilityEnum::RAINFOFIRE)
        {
            auto obj = std::make_unique<RainOfFire>(registry, entity, gameData);
            abilityMap[entity].emplace(abilityEnum, std::move(obj));
        }
        else if (abilityEnum == AbilityEnum::WHIRLWIND)
        {
            auto obj = std::make_unique<WhirlwindAbility>(registry, entity, gameData);
            abilityMap[entity].emplace(abilityEnum, std::move(obj));
        }
        else
        {
            return nullptr;
        }

        return abilityMap[entity][abilityEnum].get();
    }

    void AbilitySystem::Update()
    {
        for (auto& kv : abilityMap)
        {
            for (auto& ability : kv.second)
            {
                ability.second->Update();
            }
        }
    }

    void AbilitySystem::Draw3D()
    {
        for (auto& kv : abilityMap)
        {
            for (auto& ability : kv.second)
            {
                ability.second->Draw3D();
            }
        }
    }

    AbilitySystem::AbilitySystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }
} // namespace sage