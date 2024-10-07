//
// Created by Steve Wheeler on 27/03/2024.
//

#include "ExampleScene.hpp"
#include "GameObjectFactory.hpp"

#include "GameData.hpp"

#include "components/Collideable.hpp"
#include "components/sgTransform.hpp"

#include "Camera.hpp"
#include "Cursor.hpp"
#include "KeyMapping.hpp"
#include "Settings.hpp"

// Systems
#include "GameUiFactory.hpp"
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

#include "raylib.h"

namespace sage
{
    void ExampleScene::Update()
    {
        Scene::Update();
        data->actorMovementSystem->Update();
        data->controllableActorSystem->Update();
        data->dialogueSystem->Update();
        data->healthBarSystem->Update();
        data->stateMachines->Update();
        data->playerAbilitySystem->Update();
        data->timerSystem->Update();
        data->collisionSystem->Update();
        data->animationSystem->Update();
    }

    void ExampleScene::Draw2D()
    {
        data->dialogueSystem->Draw2D();
        Scene::Draw2D();
    }

    void ExampleScene::Draw3D()
    {
        Scene::Draw3D();
        data->healthBarSystem->Draw3D();
        data->playerAbilitySystem->Draw3D();
        data->stateMachines->Draw3D();
    }

    void ExampleScene::DrawDebug3D()
    {
        Scene::DrawDebug3D();
        data->navigationGridSystem->DrawDebug();
        data->actorMovementSystem->DrawDebug();
        data->collisionSystem->DrawDebug();
    }

    ExampleScene::ExampleScene(entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings)
        : Scene(_registry, _keyMapping, _settings)
    {
        //        lightSubSystem->lights[0] =
        //            CreateLight(LIGHT_POINT, {0, 100, 0}, Vector3Zero(), RAYWHITE, lightSubSystem->shader);

        // GameObjectFactory::createPlayer(registry, data.get(), {30.0f, 0, 20.0f}, "Player");
        GameObjectFactory::createKnight(registry, data.get(), {20.0f, 0, 20.0f}, "Knight");
        // GameObjectFactory::createPortal(registry, data.get(), Vector3{52, 0, -10});
        // GameObjectFactory::createWizardTower(registry, data.get(), Vector3{52, 0, -30});

        GameUiFactory::CreateExampleWindow(data->uiEngine.get(), {300, 200});
    }
} // namespace sage
