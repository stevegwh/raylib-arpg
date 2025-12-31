//
// Created by Steve Wheeler on 27/03/2024.
//

#include "ExampleScene.hpp"

#include "components/States.hpp"
#include "Serializer.hpp"
#include "Systems.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/DialogSystem.hpp"
#include "systems/states/StateMachines.hpp"

#include "engine/AudioManager.hpp"
#include "engine/Camera.hpp"
#include "engine/FullscreenTextOverlayManager.hpp"
#include "engine/systems/DoorSystem.hpp"
#include "engine/systems/RenderSystem.hpp"

#include "engine/Cursor.hpp"
#include "raylib.h"

namespace lq
{
    void ExampleScene::Init()
    {

        const auto soundScape = sys->audioManager->PlayMusic("resources/audio/bgs/Cave.ogg");
        SetMusicVolume(soundScape, 0.75);

        std::vector<std::pair<std::string, float>> text;
        text.emplace_back("Steve Wheeler presents...", 0.2f);
        text.emplace_back("LeverQuest", 0.2f);
        sys->fullscreenTextOverlayFactory->SetOverlay(text, 0.5f, 1.0f);

        const auto actor = sys->cursor->GetSelectedActor();
        assert(actor != entt::null);
        const auto conversationEntity = sys->renderSystem->FindRenderableByName("Opening_Dialog");
        assert(conversationEntity != entt::null);

        sys->fullscreenTextOverlayFactory->onOverlayEnding.Subscribe([actor, this]() {
            sys->stateMachines->playerStateMachine->ChangeState(actor, PlayerStateEnum::InDialog);
            auto& animationComponent = registry->get<sage::Animation>(actor);
            animationComponent.ChangeAnimationByEnum(sage::AnimationEnum::IDLE2);
        });
        auto& dialogComponent = registry->get<DialogComponent>(actor);
        dialogComponent.dialogTarget = conversationEntity;

        auto& conversationComponent = registry->get<DialogComponent>(conversationEntity);
        conversationComponent.conversation->onConversationEnd.Subscribe([this]() {
            sys->camera->FocusSelectedActor();
            // sys->audioManager->PlayMusic("resources/audio/music/bgm.ogg");
        });
    }

    ExampleScene::ExampleScene(
        entt::registry* _registry,
        sage::KeyMapping* _keyMapping,
        sage::Settings* _settings,
        sage::AudioManager* _audioManager)
        : Scene(_registry, _keyMapping, _settings, _audioManager)
    {
    }
} // namespace lq
