#pragma once

#include "PlayerStateMachine.hpp"
#include "WavemobStateMachine.hpp"
#include "game/GameStateSystem.hpp"
#include <memory>

namespace sage
{
    class GameData;
    class StateSystems
    {

      public:
        // Systems
        std::unique_ptr<WavemobStateController> wavemobStatemachine;
        std::unique_ptr<GameStateSystem> gameSystem;
        std::unique_ptr<PlayerStateController> playerStateMachine;
        void Update();
        void Draw3D();
        StateSystems(
            entt::registry* _registry,
            GameData* _gameData,
            Cursor* _cursor,
            ActorMovementSystem* _actorMovementSystem,
            CollisionSystem* _collisionSystem,
            ControllableActorSystem* _controllableActorSystem,
            NavigationGridSystem* _navigationGridSystem);
    };
} // namespace sage