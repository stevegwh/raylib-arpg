//
// Created by Steve Wheeler on 27/03/2024.
//

#include "EngineSystems.hpp"

#include "Serializer.hpp"

#include "Camera.hpp"
#include "Cursor.hpp"
#include "FullscreenTextOverlayManager.hpp"
#include "GameUiEngine.hpp"
#include "LightManager.hpp"
#include "MousePicker.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/SpatialAudioSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "systems/UberShaderSystem.hpp"
#include "UserInput.hpp"

#include <cassert>

namespace sage
{
    EngineSystems::EngineSystems(
        entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings, AudioManager* _audioManager)
        : registry(_registry),
          settings(_settings),
          audioManager(_audioManager),
          userInput(std::make_unique<UserInput>(_keyMapping, _settings)),
          camera(std::make_unique<Camera>(_registry, userInput.get(), this)),
          picker(std::make_unique<MousePicker>(_registry, this)),
          cursor(std::make_unique<Cursor>(_registry, this)),
          lightSubSystem(std::make_unique<LightManager>(_registry, camera.get())),
          transformSystem(std::make_unique<TransformSystem>(_registry)),
          renderSystem(std::make_unique<RenderSystem>(_registry)),
          collisionSystem(std::make_unique<CollisionSystem>(_registry)),
          navigationGridSystem(std::make_unique<NavigationGridSystem>(_registry, collisionSystem.get())),
          actorMovementSystem(std::make_unique<ActorMovementSystem>(_registry, this)),
          animationSystem(std::make_unique<AnimationSystem>(_registry)),
          uberShaderSystem(std::make_unique<UberShaderSystem>(_registry, this)),
          fullscreenTextOverlayFactory(std::make_unique<FullscreenTextOverlayManager>(this)),
          spatialAudioSystem(std::make_unique<SpatialAudioSystem>(_registry, this))
    {
        uiEngine = std::make_unique<GameUIEngine>(_registry, this);
    }

    EngineSystems::~EngineSystems()
    {
        // Windows unsubscribe from UserInput events during teardown, so destroy
        // the UI before UserInput's events are destroyed by member cleanup.
        uiEngine.reset();
    }

    GameUIEngine& EngineSystems::UI()
    {
        assert(uiEngine);
        return *uiEngine;
    }

    const GameUIEngine& EngineSystems::UI() const
    {
        assert(uiEngine);
        return *uiEngine;
    }

    void EngineSystems::ReplaceUiEngine(std::unique_ptr<GameUIEngine> replacement)
    {
        assert(replacement);
        uiEngine = std::move(replacement);
    }
} // namespace sage
