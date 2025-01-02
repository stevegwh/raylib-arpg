#include "Scene.hpp"

// NB: We have to include all the headers required to build GameData
#include "AbilityFactory.hpp"
#include "Camera.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/PartyMemberComponent.hpp"
#include "components/Renderable.hpp"
#include "components/Spawner.hpp"
#include "components/UberShaderComponent.hpp"
#include "Cursor.hpp"
#include "CursorClickIndicator.hpp"
#include "DialogFactory.hpp"
#include "EntityReflectionSignalRouter.hpp"
#include "FullscreenTextOverlayFactory.hpp"
#include "GameData.hpp"
#include "GameObjectFactory.hpp"
#include "GameUiEngine.hpp"
#include "GameUiFactory.hpp"
#include "ItemFactory.hpp"
#include "LightManager.hpp"
#include "NpcManager.hpp"
#include "QuestManager.hpp"
#include "Serializer.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/DialogSystem.hpp"
#include "systems/DoorSystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/HealthBarSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/states/StateMachines.hpp"
#include "systems/TimerSystem.hpp"
#include "systems/UberShaderSystem.hpp"
#include "UserInput.hpp"

#include "abilities/vfx/SpiralFountainVFX.hpp"

#include <optional>

namespace sage
{

    void Scene::Update()
    {
        data->renderSystem->Update();
        data->camera->Update();
        data->userInput->ListenForInput();
        data->cursor->Update();
        data->lightSubSystem->Update();
        data->uiEngine->Update();
        spiral->Update(GetFrameTime());
        data->cursorClickIndicator->Update();
        data->fullscreenTextOverlayFactory->Update();
    }

    void Scene::DrawDebug3D()
    {
        data->cursor->DrawDebug();
        data->camera->DrawDebug();
        data->lightSubSystem->DrawDebugLights();
    }

    void Scene::Draw3D()
    {
        data->renderSystem->Draw();
        data->cursor->Draw3D();
        // spiral->Draw3D();
    };

    void Scene::DrawDebug2D()
    {
        data->uiEngine->DrawDebug2D();
    }

    void Scene::Draw2D()
    {
        data->uiEngine->Draw2D();
        data->cursor->Draw2D();
        data->fullscreenTextOverlayFactory->Draw2D();
    }

    Scene::~Scene()
    {
        // ResourceManager::GetInstance().UnloadAll();
    }

    Scene::Scene(entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings)
        : registry(_registry), data(std::make_unique<GameData>(_registry, _keyMapping, _settings))
    {

        serializer::DeserializeJsonFile<ItemFactory>("resources/items.json", *data->itemFactory);

        auto heightMap = ResourceManager::GetInstance().GetImage("HEIGHT_MAP");
        auto normalMap = ResourceManager::GetInstance().GetImage("NORMAL_MAP");
        auto slices = heightMap.GetWidth();
        data->navigationGridSystem->Init(slices, 1.0f);

        data->navigationGridSystem->PopulateGrid(heightMap, normalMap);

        // NB: Dependent on *only* the map/static meshes having been loaded at this point
        for (const auto view = registry->view<Renderable>(); auto entity : view)
        {
            auto& uber = registry->emplace<UberShaderComponent>(
                entity, registry->get<Renderable>(entity).GetModel()->GetMaterialCount());
            uber.SetFlagAll(UberShaderComponent::Flags::Lit);
        }

        entt::entity firstPlayer = entt::null;

        const auto view = registry->view<Spawner>();
        for (auto& entity : view)
        {
            auto& spawner = registry->get<Spawner>(entity);
            if (spawner.spawnerType == SpawnerType::PLAYER)
            {
                firstPlayer =
                    GameObjectFactory::createPlayer(registry, data.get(), spawner.pos, spawner.rot, "Player");
            }
            else if (spawner.spawnerType == SpawnerType::ENEMY)
            {
                GameObjectFactory::createEnemy(registry, data.get(), spawner.pos, spawner.rot, "Goblin");
            }
            else if (spawner.spawnerType == SpawnerType::NPC)
            {
                auto npc = data->npcManager->CreateNPC(spawner.spawnerName, spawner.pos, spawner.rot);
                assert(npc != entt::null);
            }
        }

        // auto p2 = GameObjectFactory::createPlayer(registry, data.get(), Vector3Zero(), "Player 2");
        // auto p3 = GameObjectFactory::createPlayer(registry, data.get(), {10, 0, 10}, "Player 3");
        data->controllableActorSystem->SetSelectedActor(firstPlayer);

        // GameObjectFactory::createPlayer(registry, data.get(), {25, 0, 10}, "Player 3");
        // data->controllableActorSystem->SetSelectedActor(data->partySystem->GetMember(1).entity);
        //  registry->erase<Spawner>(view.begin(), view.end());

        ResourceManager::GetInstance().FontLoadFromFile(
            "resources/fonts/LibreBaskerville/LibreBaskerville-Bold.ttf");

        const auto abilityUi = GameUiFactory::CreateAbilityRow(data->uiEngine.get());
        auto w = Settings::TARGET_SCREEN_WIDTH * 0.3;
        auto h = Settings::TARGET_SCREEN_HEIGHT * 0.6;
        auto* inventoryWindow =
            GameUiFactory::CreateInventoryWindow(registry, data->uiEngine.get(), {200, 50}, w, h);
        auto* equipmentWindow =
            GameUiFactory::CreateCharacterWindow(registry, data->uiEngine.get(), {700, 50}, w, h);
        entt::sink sink{data->userInput->keyIPressed};
        sink.connect<&Window::ToggleHide>(*inventoryWindow);
        entt::sink sink2{data->userInput->keyCPressed};
        sink2.connect<&Window::ToggleHide>(*equipmentWindow);
        entt::sink sink3{data->userInput->keyFPressed};
        sink3.connect<&Camera::FocusSelectedActor>(data->camera);

        entt::sink sink4{data->userInput->keyOPressed};
        sink4.connect<[](FullscreenTextOverlayFactory& fullscreenTextOverlayFactory) {
            fullscreenTextOverlayFactory.SetOverlayTimed("You awaken in a cave.", 5.0f, 0.0f, 1.0f);
        }>(*data->fullscreenTextOverlayFactory);

        auto* window3 = GameUiFactory::CreatePartyPortraitsColumn(data->uiEngine.get());
        GameUiFactory::CreateGameWindowButtons(data->uiEngine.get(), inventoryWindow, equipmentWindow);

        spiral = std::make_unique<SpiralFountainVFX>(data.get(), nullptr);
        spiral->InitSystem();

        // Clear any CPU resources that are no longer needed
        // ResourceManager::GetInstance().UnloadImages();
        // ResourceManager::GetInstance().UnloadShaderFileText();
    };

} // namespace sage