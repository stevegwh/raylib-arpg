//
// Created by Steve Wheeler on 27/03/2024.
//

#include "ExampleScene.hpp"

#include "AudioManager.hpp"
#include "Camera.hpp"
#include "components/States.hpp"
#include "FullscreenTextOverlayFactory.hpp"
#include "Serializer.hpp"
#include "Systems.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/DialogSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/states/StateMachines.hpp"

#include "components/DoorBehaviorComponent.hpp"
#include "raylib.h"
#include "systems/DoorSystem.hpp"

namespace sage
{
    void ExampleScene::Init()
    {
        const auto soundScape = sys->audioManager->PlayMusic("resources/audio/bgs/Cave.ogg");
        SetMusicVolume(soundScape, 0.75);

        std::vector<std::pair<std::string, float>> text;
        text.emplace_back("Steve Wheeler presents...", 10.0f);
        text.emplace_back("A game by Steve Wheeler.", 3.0f);
        text.emplace_back("LeverQuest", 5.0f);
        sys->fullscreenTextOverlayFactory->SetOverlay(text, 0.5f, 1.0f);

        const auto actor = sys->controllableActorSystem->GetSelectedActor();
        const auto conversationEntity = sys->renderSystem->FindRenderableByName("Opening_Dialog");
        assert(conversationEntity != entt::null);

        std::unique_ptr<Connection> cnx =
            sys->fullscreenTextOverlayFactory->onOverlayEnding.Subscribe([actor, this]() {
                sys->stateMachines->playerStateMachine->ChangeState(actor, PlayerStateEnum::InDialog);
                auto& animationComponent = registry->get<Animation>(actor);
                animationComponent.ChangeAnimationByEnum(AnimationEnum::IDLE2);
            });

        auto& dialogComponent = registry->get<DialogComponent>(actor);
        dialogComponent.dialogTarget = conversationEntity;

        auto& conversationComponent = registry->get<DialogComponent>(conversationEntity);
        conversationComponent.conversation->onConversationEnd.Subscribe([this]() {
            sys->camera->FocusSelectedActor();
            sys->audioManager->PlayMusic("resources/audio/music/5 A Safe Space LOOP TomMusic.ogg");
        });
    }

    ExampleScene::ExampleScene(
        entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings, AudioManager* _audioManager)
        : Scene(_registry, _keyMapping, _settings, _audioManager)
    {
        auto doorId = sys->renderSystem->FindRenderable<DoorBehaviorComponent>("QUEST_DOOR");
        sys->doorSystem->UnlockAndOpenDoor(doorId);
    }
} // namespace sage
