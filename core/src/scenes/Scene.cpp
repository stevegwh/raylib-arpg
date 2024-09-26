#include "Scene.hpp"

#include "GameData.hpp"

#include "../../utils/EntityReflectionSignalRouter.hpp"

#include "Camera.hpp"
#include "Cursor.hpp"
#include "UserInput.hpp"

#include "components/Renderable.hpp"

// NB: We have to include all the headers required to build GameData
#include "AbilityFactory.hpp"
#include "Serializer.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/dialogue/DialogueSystem.hpp"
#include "systems/HealthBarSystem.hpp"
#include "systems/LightSubSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/states/StateMachines.hpp"
#include "systems/TimerSystem.hpp"

#include "components/Spawner.hpp"
#include <GameObjectFactory.hpp>

namespace sage
{

    void Scene::Update()
    {
        data->renderSystem->Update();
        data->camera->Update();
        data->userInput->ListenForInput();
        data->cursor->Update();
        data->lightSubSystem->Update();
    }

    void Scene::Draw3D()
    {
        data->renderSystem->Draw();
        // If we hit something, draw the cursor at the hit point
        data->cursor->Draw3D();
    };

    void Scene::Draw2D()
    {
        data->cursor->Draw2D();
    }

    void Scene::DrawDebug()
    {
        data->cursor->DrawDebug();
        data->camera->DrawDebug();
        data->lightSubSystem->DrawDebugLights();
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
        // registry->erase<Spawner>(view.begin(), view.end());

        // NB: Dependent on only the map/static meshes having been loaded at this point
        // Maybe time for a tag system
        for (const auto view = registry->view<Renderable>(); auto entity : view)
            data->lightSubSystem->LinkRenderableToLight(entity);

        // Clear any CPU resources that are no longer needed
        // ResourceManager::GetInstance().UnloadImages();
        // ResourceManager::GetInstance().UnloadShaderFileText();
    };

} // namespace sage