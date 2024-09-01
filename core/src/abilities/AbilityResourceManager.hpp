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

    // TODO: Move to ability factory
    class AbilityResourceManager
    {
      private:
        std::unordered_map<std::string, std::string> spellIndicators;

        AbilityResourceManager();

      public:
        std::unique_ptr<AbilityIndicator> GetIndicator(AbilityData::IndicatorData data, GameData* _gameData);
        std::unique_ptr<VisualFX> GetVisualFX(
            AbilityData::VisualFXData& data, entt::entity entity, GameData* _gameData);
        AbilityResourceManager(const AbilityResourceManager&) = delete;
        AbilityResourceManager& operator=(const AbilityResourceManager&) = delete;
        static AbilityResourceManager& GetInstance();
    };
} // namespace sage