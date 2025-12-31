//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "engine/BaseSystems.hpp"
#include "entt/entt.hpp"

// #include <memory>

namespace lq
{
    class LeverUIEngine;
    class DialogSystem;
    class DialogFactory;
    class NPCManager;
    class HealthBarSystem;
    class StateMachines;
    class AbilityFactory;
    class ItemFactory;
    class PlayerAbilitySystem;
    class CombatSystem;
    class InventorySystem;
    class PartySystem;
    class EquipmentSystem;
    class QuestManager;
    class ContextualDialogSystem;
    class SpatialAudioSystem;
    class LootTable;
    class LootSystem;
    class ControllableActorSystem;
    class CursorClickIndicator;

    class Systems : public sage::BaseSystems
    {

      public:
        std::unique_ptr<LeverUIEngine> uiEngine;
        std::unique_ptr<DialogSystem> dialogSystem;
        std::unique_ptr<DialogFactory> dialogFactory;
        std::unique_ptr<NPCManager> npcManager;
        std::unique_ptr<HealthBarSystem> healthBarSystem;
        std::unique_ptr<StateMachines> stateMachines;
        std::unique_ptr<AbilityFactory> abilityFactory;
        std::unique_ptr<ItemFactory> itemFactory;
        std::unique_ptr<PlayerAbilitySystem> playerAbilitySystem;
        std::unique_ptr<CombatSystem> combatSystem;
        std::unique_ptr<InventorySystem> inventorySystem;
        std::unique_ptr<PartySystem> partySystem;
        std::unique_ptr<EquipmentSystem> equipmentSystem;
        std::unique_ptr<CursorClickIndicator> cursorClickIndicator;
        std::unique_ptr<ControllableActorSystem> controllableActorSystem;
        std::unique_ptr<QuestManager> questManager;
        std::unique_ptr<ContextualDialogSystem> contextualDialogSystem;
        std::unique_ptr<LootTable> lootTable;
        std::unique_ptr<LootSystem> lootSystem;
        Systems(
            entt::registry* _registry,
            sage::KeyMapping* _keyMapping,
            sage::Settings* _settings,
            sage::AudioManager* _audioManager);
    };
} // namespace lq
