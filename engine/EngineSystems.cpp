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
#include "systems/ActorMovementSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/DoorSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/SpatialAudioSystem.hpp"
#include "systems/TimerSystem.hpp"
#include "systems/UberShaderSystem.hpp"
#include "UserInput.hpp"

namespace sage
{
    EngineSystems::EngineSystems(
        entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings, AudioManager* _audioManager)
        : registry(_registry),
          settings(_settings),
          audioManager(_audioManager),
          // uiEngine(std::make_unique<GameUIEngine>(_registry, this)),
          userInput(std::make_unique<UserInput>(_keyMapping, _settings)),
          cursor(std::make_unique<Cursor>(_registry, this)),
          camera(std::make_unique<Camera>(_registry, userInput.get(), this)),
          lightSubSystem(std::make_unique<LightManager>(_registry, camera.get())),
          renderSystem(std::make_unique<RenderSystem>(_registry)),
          collisionSystem(std::make_unique<CollisionSystem>(_registry)),
          navigationGridSystem(std::make_unique<NavigationGridSystem>(_registry, collisionSystem.get())),
          actorMovementSystem(std::make_unique<ActorMovementSystem>(_registry, this)),
          animationSystem(std::make_unique<AnimationSystem>(_registry)),
          timerSystem(std::make_unique<TimerSystem>(_registry)),
          uberShaderSystem(std::make_unique<UberShaderSystem>(_registry, this)),
          fullscreenTextOverlayFactory(std::make_unique<FullscreenTextOverlayManager>(this)),
          spatialAudioSystem(std::make_unique<SpatialAudioSystem>(_registry, this)),
          doorSystem(std::make_unique<DoorSystem>(_registry, this))
    {
    }
} // namespace sage
