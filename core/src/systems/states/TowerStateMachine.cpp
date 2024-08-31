#include "TowerStateMachine.hpp"

#include "GameData.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/MoveableActor.hpp"
#include "components/sgTransform.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

#include "raylib.h"

namespace sage
{
    class TowerStateController::DefaultState : public StateMachine
    {

      public:
        void Update(entt::entity entity) override
        {
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnStateEnter(entt::entity entity) override
        {
        }

        void OnStateExit(entt::entity entity) override
        {
        }

        virtual ~DefaultState() = default;

        DefaultState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
        {
        }
    };

    class TowerStateController::CombatState : public StateMachine
    {

        void onTargetDeath(entt::entity self, entt::entity target)
        {
        }

        bool checkInCombat(entt::entity entity)
        {
            return false;
        }

      public:
        void Update(entt::entity self) override
        {
        }

        void OnStateEnter(entt::entity entity) override
        {
        }

        void OnStateExit(entt::entity entity) override
        {
        }

        virtual ~CombatState() = default;

        CombatState(entt::registry* _registry, GameData* _gameData) : StateMachine(_registry, _gameData)
        {
        }
    };

    // ----------------------------

    void TowerStateController::Update()
    {
        auto view = registry->view<TowerState>();
        for (const auto& entity : view)
        {
            auto state = registry->get<TowerState>(entity).GetCurrentState();
            GetSystem(state)->Update(entity);
        }
    }

    void TowerStateController::Draw3D()
    {
        auto view = registry->view<TowerState>();
        for (const auto& entity : view)
        {
            auto state = registry->get<TowerState>(entity).GetCurrentState();
            GetSystem(state)->Draw3D(entity);
        }
    }

    TowerStateController::~TowerStateController()
    {
    }

    TowerStateController::TowerStateController(entt::registry* _registry, GameData* _gameData)
        : StateMachineController(_registry)
    {
        states[TowerStateEnum::Default] = std::make_unique<DefaultState>(_registry, _gameData);
        states[TowerStateEnum::Combat] = std::make_unique<CombatState>(_registry, _gameData);
    }
} // namespace sage