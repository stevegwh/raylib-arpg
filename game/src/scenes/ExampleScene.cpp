//
// Created by Steve Wheeler on 27/03/2024.
//

#include "ExampleScene.hpp"
#include "animation/RpgAnimationIds.hpp"

#include "MapLoader.hpp"
#include "Systems.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/DialogSystem.hpp"
#include "systems/states/PlayerStates.hpp"
#include "systems/states/StateMachines.hpp"

#include "engine/AudioManager.hpp"
#include "engine/Camera.hpp"
#include "engine/components/Animation.hpp"
#include "engine/FullscreenTextOverlayManager.hpp"
#include "engine/systems/RenderSystem.hpp"

#include "engine/Cursor.hpp"
#include "raylib.h"

namespace lq
{
    void ExampleScene::Init()
    {

        const auto soundScape = sys->engine.audioManager->PlayMusic("resources/audio/bgs/Cave.ogg");
        SetMusicVolume(soundScape, 0.75);

        std::vector<std::pair<std::string, float>> text;
        text.emplace_back("Steve Wheeler presents...", 0.2f);
        text.emplace_back("LeverQuest", 0.2f);
        sys->engine.fullscreenTextOverlayFactory->SetOverlay(text, 0.5f, 1.0f);

        const auto actor = sys->selectionSystem->GetSelectedActor();
        assert(actor != entt::null);
        const auto conversationEntity = sys->engine.renderSystem->FindRenderableByName("Opening_Dialog");
        assert(conversationEntity != entt::null);

        sys->engine.fullscreenTextOverlayFactory->onOverlayEnding.Subscribe([actor, conversationEntity, this]() {
            sys->stateMachines->playerStateMachine->ChangeState(
                actor, PlayerInDialogState{.target = conversationEntity});
            auto& animationComponent = registry->get<sage::Animation>(actor);
            animationComponent.ChangeAnimationById(lq::animation_ids::Idle2);
        });

        auto& conversationComponent = registry->get<DialogComponent>(conversationEntity);
        conversationComponent.conversation->onConversationEnd.Subscribe([this]() {
            sys->engine.camera->FocusEntity(sys->selectionSystem->GetSelectedActor());
            // sys->engine.audioManager->PlayMusic("resources/audio/music/bgm.ogg");
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
