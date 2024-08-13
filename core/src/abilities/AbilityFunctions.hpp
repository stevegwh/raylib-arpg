#pragma once

#include "particle/VisualFX.hpp"

#include "AbilityData.hpp"

#include "raylib.h"

#include <entt/entt.hpp>
#include <memory>
#include <unordered_map>

namespace sage
{
    class Camera;

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
            entt::registry* registry, entt::entity self, const AbilityData& ad) override;
    };

    class MultihitRadiusFromCursor : public AbilityFunction
    {
      public:
        void Execute(
            entt::registry* registry, entt::entity self, const AbilityData& ad) override;
    };

    class MultihitRadiusFromCaster : public AbilityFunction
    {
      public:
        void Execute(
            entt::registry* registry, entt::entity self, const AbilityData& ad) override;
    };

    enum class AbilityFunctionEnum
    {
        SingleTargetHit,
        MultihitRadiusFromCursor,
        MultihitRadiusFromCaster
    };

    class AbilityResourceManager
    {
      private:
        std::unordered_map<AbilityFunctionEnum, std::unique_ptr<AbilityFunction>>
            abilityFunctions;
        entt::registry* registry;

        AbilityResourceManager(entt::registry* reg);
        AbilityResourceManager(const AbilityResourceManager&) = delete;
        AbilityResourceManager& operator=(const AbilityResourceManager&) = delete;
        void InitializeAbilities();

      public:
        static AbilityResourceManager& GetInstance(entt::registry* reg);
        AbilityFunction* GetExecuteFunc(AbilityFunctionEnum name);
        std::unique_ptr<VisualFX> GetVisualFX(
            AbilityData::VisualFXData data, Camera* _camera);
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