#include "Scene.hpp"

#include "GameData.hpp"
#include "system_includes.hpp"

// NB: We have to include all the headers required to build GameData
#include "AudioManager.hpp"
#include "Camera.hpp"

#include "components/Renderable.hpp"
#include "components/Spawner.hpp"
#include "components/UberShaderComponent.hpp"

#include "Cursor.hpp"

#include "CursorClickIndicator.hpp"
#include "DialogFactory.hpp"
#include "FullscreenTextOverlayFactory.hpp"

#include "GameObjectFactory.hpp"
#include "GameUiEngine.hpp"
#include "GameUiFactory.hpp"
#include "Serializer.hpp"

#include "UserInput.hpp"

#include "abilities/vfx/SpiralFountainVFX.hpp"
#include "components/SpatialAudioComponent.hpp"

namespace sage
{

    void Scene::loadSpawners() const
    {
        entt::entity firstPlayer = entt::null;
        const auto spawnerView = registry->view<Spawner>();
        for (auto& entity : spawnerView)
        {
            const auto& spawner = registry->get<Spawner>(entity);
            if (spawner.type == SpawnerType::PLAYER)
            {
                firstPlayer =
                    GameObjectFactory::createPlayer(registry, data.get(), spawner.pos, spawner.rot, "Player");
            }
            else if (spawner.type == SpawnerType::ENEMY)
            {
                GameObjectFactory::createEnemy(registry, data.get(), spawner.pos, spawner.rot, "Goblin");
            }
            else if (spawner.type == SpawnerType::NPC)
            {
                auto npc = data->npcManager->CreateNPC(spawner.name, spawner.pos, spawner.rot);
                assert(npc != entt::null);
            }
            else if (spawner.type == SpawnerType::DIALOG_CUTSCENE)
            {
                GameObjectFactory::createDialogCutscene(registry, spawner.pos, spawner.name.c_str());
            }
        }
        registry->erase<Spawner>(spawnerView.begin(), spawnerView.end());
        data->controllableActorSystem->SetSelectedActor(firstPlayer);
    }

    void Scene::initAssets() const
    {
        serializer::DeserializeJsonFile<ItemFactory>("resources/items.json", *data->itemFactory);

        const auto heightMap = ResourceManager::GetInstance().GetImage("HEIGHT_MAP");
        const auto normalMap = ResourceManager::GetInstance().GetImage("NORMAL_MAP");
        const auto slices = heightMap.GetWidth();
        data->navigationGridSystem->Init(slices, 1.0f);
        data->navigationGridSystem->PopulateGrid(heightMap, normalMap);

        // NB: Dependent on *only* the map/static meshes having been loaded at this point
        for (const auto view = registry->view<Renderable>(); auto entity : view)
        {
            auto& uber = registry->emplace<UberShaderComponent>(
                entity, registry->get<Renderable>(entity).GetModel()->GetMaterialCount());
            uber.SetFlagAll(UberShaderComponent::Flags::Lit);
        }

        loadSpawners();

        // Requires renderables being loaded first
        data->contextualDialogSystem->InitContextualDialogsFromDirectory();
        data->questManager->InitQuestsFromDirectory();

        data->dialogFactory->InitDialogFromDirectory(); // Must be called after all npcs are loaded
        data->camera->FocusSelectedActor();
    }

    void Scene::initUI() const
    {
        ResourceManager::GetInstance().FontLoadFromFile(
            "resources/fonts/LibreBaskerville/LibreBaskerville-Bold.ttf");

        GameUiFactory::CreateAbilityRow(data->uiEngine.get());
        auto w = Settings::TARGET_SCREEN_WIDTH * 0.3;
        auto h = Settings::TARGET_SCREEN_HEIGHT * 0.6;
        auto* inventoryWindow =
            GameUiFactory::CreateInventoryWindow(registry, data->uiEngine.get(), {200, 50}, w, h);
        auto* equipmentWindow =
            GameUiFactory::CreateCharacterWindow(registry, data->uiEngine.get(), {700, 50}, w, h);
        data->userInput->keyIPressed.Subscribe([inventoryWindow]() { inventoryWindow->ToggleHide(); });
        data->userInput->keyCPressed.Subscribe([equipmentWindow]() { equipmentWindow->ToggleHide(); });
        data->userInput->keyFPressed.Subscribe([this]() { data->camera->FocusSelectedActor(); });

        GameUiFactory::CreatePartyPortraitsColumn(data->uiEngine.get());
        GameUiFactory::CreateGameWindowButtons(data->uiEngine.get(), inventoryWindow, equipmentWindow);
    }

    void Scene::Update()
    {
        data->audioManager->Update();
        data->renderSystem->Update();
        data->camera->Update();
        data->userInput->ListenForInput();
        data->cursor->Update();
        data->lightSubSystem->Update();
        data->uiEngine->Update();
        spiral->Update(GetFrameTime());
        data->cursorClickIndicator->Update();
        data->fullscreenTextOverlayFactory->Update();
        data->actorMovementSystem->Update();
        data->controllableActorSystem->Update();
        data->dialogSystem->Update();
        data->healthBarSystem->Update();
        data->playerAbilitySystem->Update();
        data->timerSystem->Update();
        data->collisionSystem->Update();
        data->animationSystem->Update();
        data->contextualDialogSystem->Update();
        data->spatialAudioSystem->Update();
        data->stateMachines->Update();
    }

    void Scene::DrawDebug3D()
    {
        data->cursor->DrawDebug();
        data->camera->DrawDebug();
        data->lightSubSystem->DrawDebugLights();
        data->navigationGridSystem->DrawDebug();
        data->actorMovementSystem->DrawDebug();
        data->collisionSystem->DrawDebug();
    }

    void Scene::Draw3D()
    {
        data->renderSystem->Draw();
        data->cursor->Draw3D();
        data->healthBarSystem->Draw3D();
        data->playerAbilitySystem->Draw3D();
        data->stateMachines->Draw3D();
        // spiral->Draw3D();
    };

    void Scene::DrawDebug2D()
    {
        data->uiEngine->DrawDebug2D();
    }

    void Scene::Draw2D()
    {
        data->contextualDialogSystem->Draw2D();
        data->uiEngine->Draw2D();
        data->cursor->Draw2D();
        data->fullscreenTextOverlayFactory->Draw2D();
    }

    Scene::~Scene()
    {
        // ResourceManager::GetInstance().UnloadAll();
    }

    Scene::Scene(
        entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings, AudioManager* _audioManager)
        : registry(_registry), data(std::make_unique<GameData>(_registry, _keyMapping, _settings, _audioManager))
    {

        initAssets();
        initUI();

        // Clear any CPU resources that are no longer needed
        // ResourceManager::GetInstance().UnloadImages();
        // ResourceManager::GetInstance().UnloadShaderFileText();

        // Test stuff -----------------------------
        spiral = std::make_unique<SpiralFountainVFX>(data.get(), nullptr);
        spiral->InitSystem();

        //        auto& spatial = registry->emplace<SpatialAudioComponent>(firstPlayer);
        //        spatial.audioKey = "";

        // -----------------------------------------
    };

} // namespace sage