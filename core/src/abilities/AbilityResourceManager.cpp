#include "AbilityResourceManager.hpp"

#include "GameData.hpp"

#include "vfx/VisualFX.hpp"

#include "vfx/FireballVFX.hpp"
#include "vfx/FloorFireVFX.hpp"
#include "vfx/LightningBallVFX.hpp"
#include "vfx/RainOfFireVFX.hpp"
#include "vfx/WhirlwindVFX.hpp"

#include "AbilityFunctions.hpp"
#include "AbilityIndicator.hpp"

#include "Camera.hpp"

namespace sage
{

    std::unique_ptr<AbilityIndicator> AbilityResourceManager::GetIndicator(
        AbilityData::IndicatorData data, GameData* _gameData)
    {
        // if (data.indicatorKey == "CircularCursor")
        // {
        //     return std::make_unique<AbilityIndicator>(
        //         registry,
        //         _gameData->navigationGridSystem.get(),
        //         "resources/textures/cursor/rainoffire_cursor.png");
        // }
        // return nullptr;

        return std::make_unique<AbilityIndicator>(
            _gameData->registry,
            _gameData->navigationGridSystem.get(),
            "resources/textures/cursor/rainoffire_cursor.png");
    }

    std::unique_ptr<VisualFX> AbilityResourceManager::GetVisualFX(
        AbilityData::VisualFXData& data, GameData* _gameData)
    {
        std::unique_ptr<VisualFX> obj;

        if (data.name == "RainOfFire")
        {
            obj = std::make_unique<RainOfFireVFX>(_gameData);
        }
        else if (data.name == "FloorFire")
        {
            obj = std::make_unique<FloorFireVFX>(_gameData);
        }
        else if (data.name == "360SwordSlash")
        {
            obj = std::make_unique<WhirlwindVFX>(_gameData);
        }
        else if (data.name == "LightningBall")
        {
            obj = std::make_unique<LightningBallVFX>(_gameData);
        }
        else if (data.name == "Fireball")
        {
            obj = std::make_unique<FireballVFX>(_gameData);
        }
        data.ptr = obj.get();
        return std::move(obj);
    }

    AbilityResourceManager& AbilityResourceManager::GetInstance()
    {
        static AbilityResourceManager instance;
        return instance;
    }

    AbilityResourceManager::AbilityResourceManager()
    {
    }
} // namespace sage