//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "systems/LightSubSystem.hpp"
#include "entt/entt.hpp"
#include "Game.hpp"
#include "Settings.hpp"

#include <memory>

namespace sage
{
class Scene
{
protected:
    entt::registry* registry;
    Game* game;
    Settings settings;
public:
    std::unique_ptr<LightSubSystem> lightSubSystem;
    explicit Scene(entt::registry* _registry, Game* _game, Settings _settings) :
        registry(_registry),
        game(_game),
        settings(_settings),
        lightSubSystem(std::make_unique<LightSubSystem>()) {};
    virtual ~Scene() = default;
    virtual void Update() = 0;
    virtual void Draw3D() = 0;
    virtual void Draw2D() = 0;
};

} // sage
