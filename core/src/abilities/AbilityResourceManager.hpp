#pragma once

#include "AbilityData.hpp"

#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace sage
{
    enum class AbilityFunctionEnum;
    class AbilityFunction;
    class Camera;
    class VisualFX;
    class AbilityIndicator;

    class AbilityResourceManager
    {
      private:
        entt::registry* registry;
        std::unordered_map<AbilityFunctionEnum, std::unique_ptr<AbilityFunction>> abilityFunctions;
        std::unordered_map<std::string, std::string> spellIndicators;

        AbilityResourceManager(entt::registry* reg);

        void InitializeAbilities();

      public:
        // std::unique_ptr<AbilityIndicator> GetIndicator(std::string key);
        AbilityFunctionEnum StringToExecuteFuncEnum(const std::string& name);
        AbilityFunction* GetExecuteFunc(AbilityFunctionEnum name);
        std::unique_ptr<VisualFX> GetVisualFX(AbilityData::VisualFXData data, Camera* _camera);
        AbilityResourceManager(const AbilityResourceManager&) = delete;
        AbilityResourceManager& operator=(const AbilityResourceManager&) = delete;
        static AbilityResourceManager& GetInstance(entt::registry* reg);
    };
} // namespace sage