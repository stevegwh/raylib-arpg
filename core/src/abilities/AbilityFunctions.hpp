#pragma once

#include "raylib.h"

#include <entt/entt.hpp>
#include <memory>
#include <unordered_map>

namespace sage
{
    class AbilityData;
    class AbilityFunction
    {
      protected:
      public:
        virtual ~AbilityFunction() = default;
        virtual void Execute(
            entt::registry* registry,
            entt::entity self,
            const AbilityData& abilityData) = 0;
    };

    class SingleTargetHitFunc : public AbilityFunction
    {
      public:
        void Execute(
            entt::registry* registry,
            entt::entity self,
            const AbilityData& abilityData) override;
    };

    class RainOfFireFunc : public AbilityFunction
    {
      public:
        void Execute(
            entt::registry* registry,
            entt::entity self,
            const AbilityData& abilityData) override;
    };

    class WhirlwindFunc : public AbilityFunction
    {
      public:
        void Execute(
            entt::registry* registry,
            entt::entity self,
            const AbilityData& abilityData) override;
    };

    class AbilityLibrary
    {
      private:
        std::unordered_map<std::string, std::unique_ptr<AbilityFunction>>
            abilityFunctions;
        entt::registry* registry;

        AbilityLibrary(entt::registry* reg);
        AbilityLibrary(const AbilityLibrary&) = delete;
        AbilityLibrary& operator=(const AbilityLibrary&) = delete;
        void InitializeAbilities();

      public:
        static AbilityLibrary& GetInstance(entt::registry* reg);
        AbilityFunction* GetAbility(const std::string& name);
    };

    void Hit360AroundPoint(
        entt::registry* registry,
        entt::entity self,
        AbilityData abilityData,
        Vector3 point,
        float radius);

    void HitSingleTarget(
        entt::registry* registry,
        entt::entity self,
        AbilityData abilityData,
        entt::entity target);
} // namespace sage