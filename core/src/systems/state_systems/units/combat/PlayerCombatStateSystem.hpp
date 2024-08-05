// //
// // Created by Steve on 05/06/24.
// //

// #pragma once

// #include "components/CombatableActor.hpp"
// #include "components/states/PlayerStates.hpp"
// #include "systems/state_systems/StateMachine.hpp"
// #include "systems/ControllableActorSystem.hpp"
// #include "systems/CollisionSystem.hpp"
// #include "TimerManager.hpp"

// #include "entt/entt.hpp"

// #include <memory>

// namespace sage
// {
//     class PlayerCombatStateSystem : public StateMachine<PlayerCombatStateSystem, StatePlayerCombat>
//     {
//         ControllableActorSystem* controllableActorSystem;
		
//         void startCombat(entt::entity self);
//         [[nodiscard]] bool checkInCombat(entt::entity self);
//         void onDeath(entt::entity self);
//         void onTargetDeath(entt::entity self, entt::entity target);
//         void onAttackCancel(entt::entity self);

//       public:
//         void OnEnemyClick(entt::entity self, entt::entity target);
//         void Update() override;
//         void Draw3D() override;
//         void OnHit(AttackData attackData);
//         void Enable();
//         void Disable();
//         void OnComponentAdded(entt::entity self) override;
//         void OnComponentRemoved(entt::entity self) override;

//         PlayerCombatStateSystem(entt::registry* _registry, ControllableActorSystem* _controllableActorSystem,
//                                 CollisionSystem* _collisionSystem, TimerManager* _timerManager);
//     };
// } // namespace sage
