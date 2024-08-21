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
    class GameData;
    class VisualFX;
    class AbilityIndicator;

    class AbilityResourceManager
    {
      private:
        std::unordered_map<AbilityFunctionEnum, std::unique_ptr<AbilityFunction>> abilityFunctions;
        std::unordered_map<std::string, std::string> spellIndicators;

        AbilityResourceManager();

        void InitializeAbilities();

      public:
        std::unique_ptr<AbilityIndicator> GetIndicator(AbilityData::IndicatorData data, GameData* _gameData);
        // std::unique_ptr<AbilityIndicator> GetIndicator(std::string key);
        AbilityFunctionEnum StringToExecuteFuncEnum(const std::string& name);
        AbilityFunction* GetExecuteFunc(AbilityFunctionEnum name);
        std::unique_ptr<VisualFX> GetVisualFX(AbilityData::VisualFXData data, GameData* _gameData);
        AbilityResourceManager(const AbilityResourceManager&) = delete;
        AbilityResourceManager& operator=(const AbilityResourceManager&) = delete;
        static AbilityResourceManager& GetInstance();
    };
} // namespace sage