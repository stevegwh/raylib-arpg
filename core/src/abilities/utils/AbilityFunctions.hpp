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

    class PlayerAutoAttackFunc : public AbilityFunction
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

    class WavemobAutoAttackFunc : public AbilityFunction
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

        // Private constructor
        AbilityLibrary(entt::registry* reg);

        // Delete copy constructor and assignment operator
        AbilityLibrary(const AbilityLibrary&) = delete;
        AbilityLibrary& operator=(const AbilityLibrary&) = delete;

        // Private method to initialize ability functions
        void InitializeAbilities();

      public:
        // Static method to get the instance
        static AbilityLibrary& GetInstance(entt::registry* reg);

        // Method to get an ability function
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