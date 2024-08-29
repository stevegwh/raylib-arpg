#include "AbilityIndicator.hpp"

#include "systems/NavigationGridSystem.hpp"
#include "TextureTerrainOverlay.hpp"

namespace sage
{

    void AbilityIndicator::Init(Vector3 mouseRayHit)
    {
        indicatorTexture->Init(mouseRayHit);
    }

    void AbilityIndicator::Update(Vector3 mouseRayHit)
    {
        indicatorTexture->Update(mouseRayHit);
    }

    void AbilityIndicator::Enable(bool enable)
    {
        indicatorTexture->Enable(enable);
    }

    AbilityIndicator::AbilityIndicator(
        entt::registry* _registry,
        NavigationGridSystem* _navigationGridSystem,
        const std::string& cursorTexturePath)
        : indicatorTexture(std::make_unique<TextureTerrainOverlay>(
              _registry,
              _navigationGridSystem,
              cursorTexturePath.c_str(),
              Color{255, 215, 0, 255},
              "resources/shaders/glsl330/bloom.fs"))
    {
    }
} // namespace sage