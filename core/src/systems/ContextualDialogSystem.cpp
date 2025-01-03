//
// Created by Steve Wheeler on 03/01/2025.
//

#include "ContextualDialogSystem.hpp"

#include "Camera.hpp"
#include "components/Collideable.hpp"
#include "components/ContextualDialogTriggerComponent.hpp"
#include "components/OverheadDialogComponent.hpp"
#include "components/sgTransform.hpp"
#include "ControllableActorSystem.hpp"
#include "GameData.hpp"
#include "Settings.hpp"
#include "slib.hpp"

#include "components/Renderable.hpp"
#include "raylib.h"
#include "RenderSystem.hpp"

#include <filesystem>
#include <format>
#include <fstream>

namespace fs = std::filesystem;
constexpr const auto CONTEXTUAL_DIALOG_PATH = "resources/dialog/contextual";

namespace sage
{

    void ContextualDialogSystem::initContextualDialogsFromDirectory()
    {
        for (const fs::path path{CONTEXTUAL_DIALOG_PATH}; const auto& entry : fs::directory_iterator(path))
        {
            if (entry.path().extension() == ".txt")
            {
                std::string fileName = entry.path().filename().string();
                std::ifstream file{std::format("{}/{}", CONTEXTUAL_DIALOG_PATH, fileName)};

                std::vector<std::string> text;
                auto entity = gameData->renderSystem->FindRenderableByMeshName(fileName);
                auto& trigger = registry->emplace<ContextualDialogTriggerComponent>(entity);

                if (file.is_open())
                {
                    bool metaEnd = false;
                    std::string buff;
                    while (std::getline(file, buff, '\n'))
                    {
                        if (metaEnd)
                        {
                            text.push_back(buff);
                            continue;
                        }
                        if (buff == "---")
                        {
                            metaEnd = true;
                        }
                        else if (buff.find("distance:") != std::string::npos)
                        {
                            auto sub = buff.substr(std::string("distance:").size());
                            float dist = std::stof(sub);
                            trigger.distance = dist;
                        }
                        else if (buff.find("reactee:") != std::string::npos)
                        {
                            auto sub = buff.substr(std::string("reactee:").size());
                            // Who reacts to this object (so far, only the player can).
                            // But maybe we want a goblin to say something when the player is near, etc.
                        }
                    }
                }
                else
                {
                    assert(0);
                }
                dialogTextMap.emplace(entity, text);
            }
        }
    }

    void ContextualDialogSystem::Draw2D() const
    {
        for (const auto view = registry->view<OverheadDialogComponent>(); const auto& entity : view)
        {
            const auto& col = registry->get<Collideable>(entity);
            auto [width, height] = gameData->settings->GetViewPort();
            auto center =
                Vector3MultiplyByValue(Vector3Add(col.worldBoundingBox.max, col.worldBoundingBox.min), 0.5f);
            const auto pos = Vector3{center.x, col.worldBoundingBox.max.y + 1.0f, center.z};
            auto screenPos = GetWorldToScreenEx(pos, *gameData->camera->getRaylibCam(), width, height);
            auto& contextualDiag = registry->get<OverheadDialogComponent>(entity);

            const auto text = contextualDiag.GetText();
            DrawText(
                text.c_str(),
                static_cast<int>(screenPos.x) - MeasureText(text.c_str(), 16) / 2,
                static_cast<int>(screenPos.y),
                20,
                WHITE);
        }
    }

    void ContextualDialogSystem::Update() const
    {
        const auto actor = gameData->controllableActorSystem->GetSelectedActor();
        if (registry->any_of<OverheadDialogComponent>(actor))
        {
            if (auto& overhead = registry->get<OverheadDialogComponent>(actor); overhead.IsFinished())
            {
                registry->erase<OverheadDialogComponent>(actor);
            }
            else
            {
                return;
            }
        }
        const auto actorPos = registry->get<sgTransform>(actor).GetWorldPos();
        for (const auto view = registry->view<sgTransform, ContextualDialogTriggerComponent, Renderable>();
             auto entity : view)
        {
            const auto& trans = registry->get<sgTransform>(entity);
            if (auto& trigger = registry->get<ContextualDialogTriggerComponent>(entity);
                trigger.CanTrigger() && Vector3Distance(trans.GetWorldPos(), actorPos) < trigger.distance)
            {
                auto& overhead = registry->emplace<OverheadDialogComponent>(actor);
                const auto contextualDialog = dialogTextMap.at(entity);
                overhead.SetText(contextualDialog, 3.0f);
                trigger.SetTriggered();
            }
        }
    }

    ContextualDialogSystem::ContextualDialogSystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
        initContextualDialogsFromDirectory();
    }

} // namespace sage