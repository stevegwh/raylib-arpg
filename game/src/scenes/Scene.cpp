#include "Scene.hpp"

#include "system_includes.hpp"
#include "Systems.hpp"

#include "../CursorClickIndicator.hpp"
#include "engine/AudioManager.hpp"
#include "engine/Camera.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/Spawner.hpp"
#include "engine/components/UberShaderComponent.hpp"
#include "engine/Cursor.hpp"
#include "engine/FullscreenTextOverlayManager.hpp"
#include "engine/GameUiEngine.hpp"

#include "DialogFactory.hpp"
#include "engine/GameUiEngine.hpp"
#include "GameObjectFactory.hpp"
#include "GameUI.hpp"
#include "GameUiFactory.hpp"
#include "Serializer.hpp"

#include "engine/UserInput.hpp"

#include "abilities/vfx/SpiralFountainVFX.hpp"
#include "engine/components/SpatialAudioComponent.hpp"

namespace lq
{

    void Scene::loadSpawners() const
    {
        entt::entity firstPlayer = entt::null;
        const auto spawnerView = registry->view<sage::Spawner>();
        for (auto& entity : spawnerView)
        {
            const auto& spawner = registry->get<sage::Spawner>(entity);
            if (spawner.type == sage::SpawnerType::PLAYER)
            {
                firstPlayer =
                    GameObjectFactory::createPlayer(registry, sys.get(), spawner.pos, spawner.rot, "Player");
            }
            else if (spawner.type == sage::SpawnerType::ENEMY)
            {
                GameObjectFactory::createEnemy(registry, sys.get(), spawner.pos, spawner.rot, "Goblin");
            }
            else if (spawner.type == sage::SpawnerType::NPC)
            {
                auto npc = sys->npcManager->CreateNPC(spawner.name, spawner.pos, spawner.rot);
                assert(npc != entt::null);
            }
            else if (spawner.type == sage::SpawnerType::DIALOG_CUTSCENE)
            {
                GameObjectFactory::createDialogCutscene(registry, spawner.pos, spawner.name.c_str());
            }
        }
        registry->erase<sage::Spawner>(spawnerView.begin(), spawnerView.end());
        sys->cursor->SetSelectedActor(firstPlayer);
    }

    void Scene::initAssets() const
    {
        sage::serializer::DeserializeJsonFile<ItemFactory>("resources/items.json", *sys->itemFactory);
        sage::serializer::DeserializeJsonFile<LootTable>("resources/loot-table.json", *sys->lootTable);
        // serializer::SaveClassJson<LootTable>("resources/loot-table.json", *sys->lootTable);

        const auto heightMap = sage::ResourceManager::GetInstance().GetImage("HEIGHT_MAP");
        const auto normalMap = sage::ResourceManager::GetInstance().GetImage("NORMAL_MAP");
        const auto slices = heightMap.GetWidth();
        sys->navigationGridSystem->Init(slices, 1.0f);
        sys->navigationGridSystem->PopulateGrid(heightMap, normalMap);

        // NB: Dependent on *only* the map/static meshes having been loaded at this point
        for (const auto view = registry->view<sage::Renderable>(); auto entity : view)
        {
            auto& uber = registry->emplace<sage::UberShaderComponent>(
                entity, registry->get<sage::Renderable>(entity).GetModel()->GetMaterialCount());
            uber.SetFlagAll(sage::UberShaderComponent::Flags::Lit);
        }

        loadSpawners();

        // Requires renderables being loaded first
        sys->contextualDialogSystem->InitContextualDialogsFromDirectory();
        sys->questManager->InitQuestsFromDirectory();

        sys->dialogFactory->InitDialogFromDirectory(); // Must be called after all npcs are loaded
        sys->camera->FocusSelectedActor();
    }

    void Scene::initUI() const
    {
        GameUiFactory::CreateAbilityRow(sys->uiEngine.get());
        auto w = sage::Settings::TARGET_SCREEN_WIDTH * 0.3;
        auto h = sage::Settings::TARGET_SCREEN_HEIGHT * 0.6;
        auto* inventoryWindow =
            GameUiFactory::CreateInventoryWindow(registry, sys->uiEngine.get(), {200, 50}, w, h);
        auto* equipmentWindow =
            GameUiFactory::CreateCharacterWindow(registry, sys->uiEngine.get(), {700, 50}, w, h);
        auto* journalWindow = GameUiFactory::CreateJournalWindow(registry, sys->uiEngine.get(), {900, 150}, w, h);

        sys->userInput->keyIPressed.Subscribe([inventoryWindow]() { inventoryWindow->ToggleHide(); });
        sys->userInput->keyCPressed.Subscribe([equipmentWindow]() { equipmentWindow->ToggleHide(); });
        sys->userInput->keyJPressed.Subscribe([journalWindow]() { journalWindow->ToggleHide(); });

        sys->userInput->keyFPressed.Subscribe([this]() { sys->camera->FocusSelectedActor(); });

        GameUiFactory::CreatePartyPortraitsColumn(sys->uiEngine.get());
        GameUiFactory::CreateGameWindowButtons(
            sys->uiEngine.get(), inventoryWindow, equipmentWindow, journalWindow);
    }

    void Scene::Update()
    {
        sys->audioManager->Update();
        sys->renderSystem->Update();
        sys->camera->Update();
        sys->userInput->ListenForInput();
        sys->cursor->Update();
        sys->lightSubSystem->Update();
        sys->uiEngine->Update();
        spiral->Update(GetFrameTime());
        sys->cursorClickIndicator->Update();
        sys->fullscreenTextOverlayFactory->Update();
        sys->actorMovementSystem->Update();
        sys->controllableActorSystem->Update();
        sys->healthBarSystem->Update();
        sys->timerSystem->Update();
        sys->collisionSystem->Update();
        sys->animationSystem->Update();
        sys->contextualDialogSystem->Update();
        sys->spatialAudioSystem->Update();
        sys->lootSystem->Update();
        sys->stateMachines->Update();
    }

    void Scene::DrawDebug3D()
    {
        sys->cursor->DrawDebug();
        sys->camera->DrawDebug();
        sys->lightSubSystem->DrawDebugLights();
        sys->navigationGridSystem->DrawDebug();
        sys->actorMovementSystem->DrawDebug();
        sys->collisionSystem->DrawDebug();
    }

    void Scene::Draw3D()
    {
        sys->renderSystem->Draw();
        sys->cursor->Draw3D();
        sys->healthBarSystem->Draw3D();
        sys->stateMachines->Draw3D();
        // spiral->Draw3D();
    };

    void Scene::DrawDebug2D()
    {
        sys->uiEngine->DrawDebug2D();
    }

    void Scene::Draw2D()
    {
        sys->contextualDialogSystem->Draw2D();
        sys->uiEngine->Draw2D();
        sys->cursor->Draw2D();
        sys->fullscreenTextOverlayFactory->Draw2D();
    }

    Scene::~Scene()
    {
        // ResourceManager::GetInstance().UnloadAll();
    }

    Scene::Scene(
        entt::registry* _registry,
        sage::KeyMapping* _keyMapping,
        sage::Settings* _settings,
        sage::AudioManager* _audioManager)
        : registry(_registry), sys(std::make_unique<Systems>(_registry, _keyMapping, _settings, _audioManager))
    {

        initAssets();
        initUI();

        // Clear any CPU resources that are no longer needed
        // ResourceManager::GetInstance().UnloadImages();
        // ResourceManager::GetInstance().UnloadShaderFileText();

        // Test stuff -----------------------------
        spiral = std::make_unique<SpiralFountainVFX>(sys.get(), nullptr);
        spiral->InitSystem();

        //        auto& spatial = registry->emplace<SpatialAudioComponent>(firstPlayer);
        //        spatial.audioKey = "";

        // -----------------------------------------
    };

} // namespace lq