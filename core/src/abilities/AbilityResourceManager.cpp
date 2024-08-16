#include "AbilityResourceManager.hpp"

#include "vfx/RainOfFireVFX.hpp"
#include "vfx/VisualFX.hpp"

#include "AbilityFunctions.hpp"

#include "Camera.hpp"

namespace sage
{

    void AbilityResourceManager::InitializeAbilities()
    {
        abilityFunctions.emplace(AbilityFunctionEnum::SingleTargetHit, std::make_unique<SingleTargetHitFunc>());
        abilityFunctions.emplace(
            AbilityFunctionEnum::MultihitRadiusFromCursor, std::make_unique<MultihitRadiusFromCursor>());
        abilityFunctions.emplace(
            AbilityFunctionEnum::MultihitRadiusFromCaster, std::make_unique<MultihitRadiusFromCaster>());
    }

    std::unique_ptr<VisualFX> AbilityResourceManager::GetVisualFX(AbilityData::VisualFXData data, Camera* _camera)
    {
        if (data.name == "RainOfFire")
        {
            auto obj = std::make_unique<RainOfFireVFX>(_camera->getRaylibCam());
            data.ptr = obj.get();
            return std::move(obj);
        }

        return nullptr;
    }

    AbilityFunctionEnum AbilityResourceManager::StringToExecuteFuncEnum(const std::string& name)
    {
        if (name == "SingleTargetHit")
        {
            return AbilityFunctionEnum::SingleTargetHit;
        }
        else if (name == "MultihitRadiusFromCaster")
        {
            return AbilityFunctionEnum::MultihitRadiusFromCaster;
        }
        else if (name == "MultihitRadiusFromCursor")
        {
            return AbilityFunctionEnum::MultihitRadiusFromCursor;
        }
        return AbilityFunctionEnum::SingleTargetHit; // TODO: Null?
    }

    AbilityFunction* AbilityResourceManager::GetExecuteFunc(AbilityFunctionEnum name)
    {
        if (abilityFunctions.empty())
        {
            InitializeAbilities();
        }
        auto it = abilityFunctions.find(name);
        return (it != abilityFunctions.end()) ? it->second.get() : nullptr;
    }

    AbilityResourceManager& AbilityResourceManager::GetInstance(entt::registry* reg)
    {
        static AbilityResourceManager instance(reg);
        return instance;
    }

    AbilityResourceManager::AbilityResourceManager(entt::registry* reg) : registry(reg)
    {
    }
} // namespace sage