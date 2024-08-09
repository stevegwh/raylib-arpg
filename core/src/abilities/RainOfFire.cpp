#include "RainOfFire.hpp"

static constexpr float COOLDOWN = 3.0f;
static constexpr float DELAY = 0.75f;
static constexpr int DAMAGE = 25;

namespace sage
{

    static constexpr AbilityData _abilityData{
        .cooldownDuration = COOLDOWN,
        .range = 5,
        .baseDamage = DAMAGE,
        .element = AttackElement::FIRE,
        .animationDelay = DELAY};

    RainOfFire::RainOfFire(
        entt::registry* _registry,
        Camera* _camera,
        Cursor* _cursor,
        NavigationGridSystem* _navigationGridSystem)
        : CursorAbility(
              _registry,
              _camera,
              _cursor,
              std::make_unique<TextureTerrainOverlay>(
                  _registry,
                  _navigationGridSystem,
                  "resources/textures/cursor/rainoffire_cursor.png",
                  Color{255, 215, 0, 255},
                  "resources/shaders/glsl330/bloom.fs"),
              _abilityData)
    {
    }
} // namespace sage