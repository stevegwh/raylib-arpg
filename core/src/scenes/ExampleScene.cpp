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
#include "components/QuestComponents.hpp"
#include "DialogFactory.hpp"
#include "GameUiFactory.hpp"
#include "ItemFactory.hpp"
#include "LightManager.hpp"
#include "QuestManager.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/DialogSystem.hpp"
#include "systems/HealthBarSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/states/StateMachines.hpp"
#include "systems/TimerSystem.hpp"

#include <Serializer.hpp>

#include "raylib.h"
#include "ViewSerializer.hpp"

namespace sage
{
    void ExampleScene::Update()
    {
        Scene::Update();
        data->actorMovementSystem->Update();
        data->controllableActorSystem->Update();
        data->dialogSystem->Update();
        data->healthBarSystem->Update();
        data->stateMachines->Update();
        data->playerAbilitySystem->Update();
        data->timerSystem->Update();
        data->collisionSystem->Update();
        data->animationSystem->Update();
    }

    void ExampleScene::Draw2D()
    {
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
        // GameObjectFactory::createArissa(registry, data.get(), {0.0f, 0, 25.0f}, {0, 0, 0});
        // data->questManager->GetQuest("ArissaQuest");

        // GameObjectFactory::createQuestNPC(registry, data.get(), {10.0f, 0, 25.0f}, "Quest NPC");

        //        {
        //            auto item = data->itemFactory->GetItem("Dagger");
        //            GameObjectFactory::spawnItemInWorld(registry, data.get(), item, {0, 0, 0});
        //            auto quest2Id = data->questManager->GetQuest("ItemFetchQuest");
        //            registry->emplace<QuestTaskComponent>(item, "ItemFetchQuest");
        //            auto& quest = registry->get<Quest>(quest2Id);
        //            quest.AddTask(item);
        //            GameObjectFactory::createFetchQuestNPC(registry, data.get(), {-10.0f, 0, 0}, "Fetch Quest
        //            NPC"); quest.StartQuest();
        //        }

        data->dialogFactory->LoadDialog(); // Must be called after all npcs are loaded
    }
} // namespace sage
