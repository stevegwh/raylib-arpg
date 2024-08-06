#include "StateSystems.hpp"

namespace sage
{
    void StateSystems::Update()
    {
        wavemobStatemachine->Update();
        gameSystem->Update();
        playerStateMachine->Update();
    }

    void StateSystems::Draw3D()
    {
        wavemobStatemachine->Draw3D();
        playerStateMachine->Draw3D();
        // gameSystem->Draw3D();
    }

    StateSystems::StateSystems(
        entt::registry* _registry,
        GameData* _gameData,
        Cursor* _cursor,
        ActorMovementSystem* _actorMovementSystem,
        CollisionSystem* _collisionSystem,
        ControllableActorSystem* _controllableActorSystem,
        NavigationGridSystem* _navigationGridSystem)
    {

        wavemobStatemachine = std::make_unique<WavemobStateController>(
            _registry,
            _cursor,
            _controllableActorSystem,
            _actorMovementSystem,
            _collisionSystem,
            _navigationGridSystem);
        playerStateMachine = std::make_unique<PlayerStateController>(
            _registry,
            _cursor,
            _controllableActorSystem,
            _actorMovementSystem,
            _collisionSystem,
            _navigationGridSystem);

        gameSystem = std::make_unique<GameStateSystem>(_registry, _gameData);
    }
} // namespace sage