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
#include "GameUiFactory.hpp"
#include "LightManager.hpp"
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

#include "raylib.h"

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

        // Quests should have a manager (a hash map, basically) so we can choose a consistent identifier (a string)
        // that will fetch the entity id for the quest Quests should all be initialised (i.e., their entity
        // created), before spawning any actors. This stops us having to pass the quest ids as a parameter.
        auto questEntity = registry->create();
        auto& quest = registry->emplace<Quest>(questEntity, registry, questEntity);

        auto knightId =
            GameObjectFactory::createKnight(registry, data.get(), {20.0f, 0, 20.0f}, "Knight", questEntity);

        auto talkTaskEntity =
            GameObjectFactory::createQuestNPC(registry, data.get(), {10.0f, 0, 25.0f}, "Quest NPC", questEntity);
    }
} // namespace sage
