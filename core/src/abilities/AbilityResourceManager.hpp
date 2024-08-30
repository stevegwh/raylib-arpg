#pragma once

#include "AbilityData.hpp"

#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace sage
{
    class GameData;
    class VisualFX;
    class AbilityIndicator;

    class AbilityResourceManager
    {
      private:
        std::unordered_map<AbilityFunctionEnum, std::unique_ptr<AbilityFunction>> abilityFunctions;
        std::unordered_map<std::string, std::string> spellIndicators;

        AbilityResourceManager();

      public:
        AbilityIndicator* GetIndicator(AbilityData::IndicatorData data, GameData* _gameData);
        // std::unique_ptr<AbilityIndicator> GetIndicator(std::string key);
        VisualFX* GetVisualFX(AbilityData::VisualFXData& data, entt::entity entity, GameData* _gameData);
        AbilityResourceManager(const AbilityResourceManager&) = delete;
        AbilityResourceManager& operator=(const AbilityResourceManager&) = delete;
        static AbilityResourceManager& GetInstance();
    };
} // namespace sage