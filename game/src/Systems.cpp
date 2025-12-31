//
// Created by Steve Wheeler on 27/03/2024.
//

#include "Systems.hpp"

#include "GameUI.hpp"

#include "engine/Camera.hpp"
#include "system_includes.hpp"

namespace lq
{
    Systems::Systems(
        entt::registry* _registry,
        sage::KeyMapping* _keyMapping,
        sage::Settings* _settings,
        sage::AudioManager* _audioManager)
        : BaseSystems(_registry, _keyMapping, _settings, _audioManager),
          uiEngine(std::make_unique<LeverUIEngine>(_registry, this)),
          cursorClickIndicator(std::make_unique<CursorClickIndicator>(_registry, this)),
          dialogSystem(std::make_unique<DialogSystem>(_registry, this)),
          dialogFactory(std::make_unique<DialogFactory>(_registry, this)),
          npcManager(std::make_unique<NPCManager>(_registry, this)),
          healthBarSystem(std::make_unique<HealthBarSystem>(_registry, camera.get())),
          stateMachines(std::make_unique<StateMachines>(_registry, this)),
          abilityFactory(std::make_unique<AbilityFactory>(_registry, this)),
          itemFactory(std::make_unique<ItemFactory>(_registry)),
          playerAbilitySystem(std::make_unique<PlayerAbilitySystem>(_registry, this)),
          combatSystem(std::make_unique<CombatSystem>(_registry)),
          inventorySystem(std::make_unique<InventorySystem>(_registry, this)),
          partySystem(std::make_unique<PartySystem>(_registry, this)),
          equipmentSystem(std::make_unique<EquipmentSystem>(_registry, this)),
          controllableActorSystem(std::make_unique<ControllableActorSystem>(_registry, this)),
          questManager(std::make_unique<QuestManager>(_registry, this)),
          contextualDialogSystem(std::make_unique<ContextualDialogSystem>(_registry, this)),
          lootTable(std::make_unique<LootTable>(_registry, this)),
          lootSystem(std::make_unique<LootSystem>(_registry, this))
    {
        BaseSystems::uiEngine = this->uiEngine.get();
    }
} // namespace lq
