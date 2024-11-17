#include "Scene.hpp"

#include "AbilityFactory.hpp"
#include "Camera.hpp"
#include "components/PartyMemberComponent.hpp"
#include "components/Renderable.hpp"
#include "components/Spawner.hpp"
#include "Cursor.hpp"
#include "EntityReflectionSignalRouter.hpp"
#include "GameData.hpp"
#include "GameObjectFactory.hpp"
#include "GameUiEngine.hpp"
#include "GameUiFactory.hpp"
#include "Serializer.hpp"
#include "UserInput.hpp"

// NB: We have to include all the headers required to build GameData
#include "components/EquipmentComponent.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/DialogSystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/HealthBarSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/LightSubSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/states/StateMachines.hpp"
#include "systems/TimerSystem.hpp"

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
        // If we hit something, draw the cursor at the hit point
        data->cursor->Draw3D();
    };

    void Scene::DrawDebug2D()
    {
        data->uiEngine->DrawDebug2D();
    }

    void Scene::Draw2D()
    {
        data->uiEngine->Draw2D();
        data->cursor->Draw2D();
    }

    Scene::~Scene()
    {
        // ResourceManager::GetInstance().UnloadAll();
    }

    Scene::Scene(entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings)
        : registry(_registry), data(std::make_unique<GameData>(_registry, _keyMapping, _settings))
    {

        // TODO: This is calculated during the map construction process. Need to find a way of reading that data,
        // instead of a magic number
        float slices = 500;
        data->navigationGridSystem->Init(slices, 1.0f);

        auto heightMap = ResourceManager::GetInstance().GetImage(AssetID::GEN_IMG_HEIGHTMAP);
        auto normalMap = ResourceManager::GetInstance().GetImage(AssetID::GEN_IMG_NORMALMAP);
        data->navigationGridSystem->PopulateGrid(heightMap, normalMap);

        // NB: Dependent on only the map/static meshes having been loaded at this point
        // Maybe time for a tag system
        for (const auto view = registry->view<Renderable>(); auto entity : view)
            data->lightSubSystem->LinkRenderableToLight(entity);

        const auto view = registry->view<Spawner>();
        for (auto& entity : view)
        {
            auto& spawner = registry->get<Spawner>(entity);
            if (spawner.spawnerType == SpawnerType::PLAYER)
            {
                GameObjectFactory::createPlayer(registry, data.get(), spawner.pos, "Player");
            }
            else if (spawner.spawnerType == SpawnerType::GOBLIN)
            {
                GameObjectFactory::createEnemy(registry, data.get(), spawner.pos, "Goblin");
            }
        }

        // GameObjectFactory::createPlayer(registry, data.get(), Vector3Zero(), "Player 2");
        // GameObjectFactory::createPlayer(registry, data.get(), {10, 0, 10}, "Player 3");
        // data->controllableActorSystem->SetSelectedActor(data->partySystem->GetMember(1).entity);
        //  registry->erase<Spawner>(view.begin(), view.end());

        // Clear any CPU resources that are no longer needed
        // ResourceManager::GetInstance().UnloadImages();
        // ResourceManager::GetInstance().UnloadShaderFileText();

        ResourceManager::GetInstance().FontLoadFromFile(
            "resources/fonts/LibreBaskerville/LibreBaskerville-Bold.ttf");
        const auto abilityUi = GameUiFactory::CreateAbilityRow(data->uiEngine.get());
        auto w = data->settings->TARGET_SCREEN_WIDTH * 0.3;
        auto h = data->settings->TARGET_SCREEN_HEIGHT * 0.6;
        auto* window = GameUiFactory::CreateInventoryWindow(registry, data->uiEngine.get(), {200, 50}, w, h);
        auto* window2 = GameUiFactory::CreateCharacterWindow(registry, data->uiEngine.get(), {700, 50}, w, h);
        entt::sink sink{data->userInput->keyIPressed};
        sink.connect<&Window::ToggleHide>(*window);
        entt::sink sink2{data->userInput->keyCPressed};
        sink2.connect<&Window::ToggleHide>(*window2);

        auto* window3 = GameUiFactory::CreatePartyPortraitsColumn(data->uiEngine.get());
    };

} // namespace sage