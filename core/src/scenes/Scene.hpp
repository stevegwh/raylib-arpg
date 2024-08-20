//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "GameData.hpp"
#include "systems/LightSubSystem.hpp"
#include <GameObjectFactory.hpp>

#include "Camera.hpp"
#include "Cursor.hpp"
#include "Settings.hpp"
#include "UserInput.hpp"

// Systems
#include "systems/ActorMovementSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/dialogue/DialogueSystem.hpp"
#include "systems/HealthBarSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/states/StateMachines.hpp"

#include <memory>

namespace sage
{
    class Scene
    {
      protected:
        entt::registry* registry;

      public:
        std::unique_ptr<GameData> data;
        std::unique_ptr<LightSubSystem> lightSubSystem;
        entt::sigh<void()> sceneChange;

        virtual void Update();

        virtual void Draw3D();

        virtual void Draw2D();

        virtual ~Scene();

        virtual void DrawDebug();

        explicit Scene(entt::registry* _registry, std::unique_ptr<GameData> _data, const std::string& mapPath);
    };
} // namespace sage
