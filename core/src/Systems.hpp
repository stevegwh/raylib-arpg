//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "entt/entt.hpp"

// #include <memory>

namespace sage
{
    // Core structs/classes
    struct Settings;
    struct KeyMapping;
    class AudioManager;

    // Input/UI group
    class UserInput;
    class Cursor;
    class Camera;
    class LightManager;
    class GameUIEngine;

    // Systems group
    class RenderSystem;
    class CollisionSystem;
    class NavigationGridSystem;
    class ActorMovementSystem;
    class ControllableActorSystem;
    class AnimationSystem;
    class DialogSystem;
    class DialogFactory;
    class NPCManager;
    class HealthBarSystem;
    class StateMachines;
    class AbilityFactory;
    class ItemFactory;
    class PlayerAbilitySystem;
    class CombatSystem;
    class TimerSystem;
    class InventorySystem;
    class PartySystem;
    class EquipmentSystem;
    class UberShaderSystem;
    class CursorClickIndicator;
    class QuestManager;
    class DoorSystem;
    class FullscreenTextOverlayManager;
    class ContextualDialogSystem;
    class SpatialAudioSystem;
    class LootTable;
    class LootSystem;

    class Systems
    {
      public:
        entt::registry* registry;
        Settings* settings;
        AudioManager* audioManager;

        std::unique_ptr<UserInput> userInput;
        std::unique_ptr<Cursor> cursor;
        std::unique_ptr<Camera> camera;
        std::unique_ptr<LightManager> lightSubSystem;
        std::unique_ptr<GameUIEngine> uiEngine;

        std::unique_ptr<RenderSystem> renderSystem;
        std::unique_ptr<CollisionSystem> collisionSystem;
        std::unique_ptr<NavigationGridSystem> navigationGridSystem;
        std::unique_ptr<ActorMovementSystem> actorMovementSystem;
        std::unique_ptr<ControllableActorSystem> controllableActorSystem;
        std::unique_ptr<AnimationSystem> animationSystem;
        std::unique_ptr<DialogSystem> dialogSystem;
        std::unique_ptr<DialogFactory> dialogFactory;
        std::unique_ptr<NPCManager> npcManager;
        std::unique_ptr<HealthBarSystem> healthBarSystem;
        std::unique_ptr<StateMachines> stateMachines;
        std::unique_ptr<AbilityFactory> abilityFactory;
        std::unique_ptr<ItemFactory> itemFactory;
        std::unique_ptr<PlayerAbilitySystem> playerAbilitySystem;
        std::unique_ptr<CombatSystem> combatSystem;
        std::unique_ptr<TimerSystem> timerSystem;
        std::unique_ptr<InventorySystem> inventorySystem;
        std::unique_ptr<PartySystem> partySystem;
        std::unique_ptr<EquipmentSystem> equipmentSystem;
        std::unique_ptr<UberShaderSystem> uberShaderSystem;
        std::unique_ptr<CursorClickIndicator> cursorClickIndicator;
        std::unique_ptr<QuestManager> questManager;
        std::unique_ptr<DoorSystem> doorSystem;
        std::unique_ptr<FullscreenTextOverlayManager> fullscreenTextOverlayFactory;
        std::unique_ptr<ContextualDialogSystem> contextualDialogSystem;
        std::unique_ptr<SpatialAudioSystem> spatialAudioSystem;
        std::unique_ptr<LootTable> lootTable;
        std::unique_ptr<LootSystem> lootSystem;

        Systems(
            entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings, AudioManager* _audioManager);
    };
} // namespace sage
