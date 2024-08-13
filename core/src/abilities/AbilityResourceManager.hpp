#pragma once

#include "particle/VisualFX.hpp"

#include "AbilityData.hpp"

#include "raylib.h"

#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace sage
{
    enum class AbilityFunctionEnum;
    class AbilityFunction;
    class Camera;

    class AbilityResourceManager
    {
      private:
        std::unordered_map<AbilityFunctionEnum, std::unique_ptr<AbilityFunction>> abilityFunctions;
        entt::registry* registry;

        AbilityResourceManager(entt::registry* reg);

        void InitializeAbilities();

      public:
        AbilityResourceManager(const AbilityResourceManager&) = delete;
        AbilityResourceManager& operator=(const AbilityResourceManager&) = delete;

        AbilityFunctionEnum StringToExecuteFuncEnum(const std::string& name);
        AbilityFunction* GetExecuteFunc(AbilityFunctionEnum name);
        static AbilityResourceManager& GetInstance(entt::registry* reg);
        std::unique_ptr<VisualFX> GetVisualFX(AbilityData::VisualFXData data, Camera* _camera);
    };
} // namespace sage