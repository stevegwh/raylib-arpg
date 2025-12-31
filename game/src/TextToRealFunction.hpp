//
// Created by steve on 15/01/2025.
//

#pragma once

#include "engine/AudioManager.hpp"
#include "engine/Camera.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/DoorBehaviorComponent.hpp"
#include "engine/components/OverheadDialogComponent.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/FullscreenTextOverlayManager.hpp"
#include "engine/Settings.hpp"
#include "engine/slib.hpp"
#include "engine/systems/DoorSystem.hpp"
#include "engine/systems/RenderSystem.hpp"

#include "components/ContextualDialogTriggerComponent.hpp"
#include "components/DialogComponent.hpp"
#include "components/ItemComponent.hpp"
#include "ParsingHelpers.hpp"
#include "Systems.hpp"
#include "systems/PartySystem.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

namespace lq::parsing
{
    template <typename EventType, typename... Args>
    void BindFunctionToEvent(entt::registry* registry, Systems* sys, TextFunction func, EventType* event)
    {
        assert(!func.name.empty());
        // Not all functions require params

        if (func.name.find("OpenDoor") != std::string::npos)
        {
            assert(!func.params.empty());
            auto doorId = sys->renderSystem->FindRenderable<sage::DoorBehaviorComponent>(func.params);
            assert(doorId != entt::null);
            event->Subscribe([doorId, sys](Args...) { sys->doorSystem->UnlockAndOpenDoor(doorId); });
        }
        else if (func.name.find("JoinParty") != std::string::npos)
        {
            assert(!func.params.empty());
            auto npcId = sys->renderSystem->FindRenderable(func.params);
            assert(npcId != entt::null);
            event->Subscribe([npcId, sys](Args...) { sys->partySystem->NPCToMember(npcId); });
        }
        else if (func.name.find("RemoveItem") != std::string::npos)
        {
            assert(!func.params.empty());
            event->Subscribe(
                [params = func.params, sys](Args...) { sys->partySystem->RemoveItemFromParty(params); });
        }
        else if (func.name.find("GiveItem") != std::string::npos)
        {
            assert(!func.params.empty());
            event->Subscribe(
                [itemName = func.params, sys](Args...) { sys->partySystem->GiveItemToSelected(itemName); });
        }
        else if (func.name.find("PlaySFX") != std::string::npos)
        {
            assert(!func.params.empty());
            event->Subscribe([sfxName = func.params, sys](Args...) { sys->audioManager->PlaySFX(sfxName); });
        }
        else if (func.name.find("PlayMusic") != std::string::npos)
        {
            assert(!func.params.empty());
            event->Subscribe([musicName = func.params, sys](Args...) { sys->audioManager->PlayMusic(musicName); });
        }
        else if (func.name.find("DisableWorldItem") != std::string::npos)
        {
            assert(!func.params.empty());
            auto itemId = sys->renderSystem->FindRenderable(func.params);
            assert(itemId != entt::null);
            event->Subscribe([itemId, registry](Args...) {
                if (registry->any_of<sage::Renderable>(itemId))
                {
                    registry->get<sage::Renderable>(itemId).Disable();
                }
                if (registry->any_of<sage::Collideable>(itemId))
                {
                    registry->get<sage::Collideable>(itemId).Disable();
                }
            });
        }
        else if (func.name.find("EndGame") != std::string::npos)
        {
            event->Subscribe([sys](Args...) {
                std::vector<std::pair<std::string, float>> text;
                text.emplace_back("Our bold heroes step forward, out of the gate.", 4.0f);
                text.emplace_back("What will await our valiant heroes?", 4.0f);
                text.emplace_back("Find out soon.", 4.0f);
                text.emplace_back("Thanks for playing!", 4.0f);
                sys->fullscreenTextOverlayFactory->SetOverlay(text, 0.5f, 1.0f);
                sys->fullscreenTextOverlayFactory->onOverlayEnd.Subscribe([sys] { sys->settings->ExitProgram(); });
            });
        }
        else
        {
            assert(0);
        }
    }
} // namespace lq::parsing
