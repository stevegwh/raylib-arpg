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
constexpr auto CONTEXTUAL_DIALOG_PATH = "resources/dialog/contextual";

namespace sage
{

    void ContextualDialogSystem::InitContextualDialogsFromDirectory()
    {
        for (const fs::path path{CONTEXTUAL_DIALOG_PATH}; const auto& entry : fs::directory_iterator(path))
        {
            if (entry.path().extension() == ".txt")
            {
                std::string fileName = entry.path().filename().string();
                std::ifstream file{std::format("{}/{}", CONTEXTUAL_DIALOG_PATH, fileName)};

                std::string fileNameStripped = entry.path().filename().replace_extension("").string();
                auto entity = gameData->renderSystem->FindRenderable(fileNameStripped);
                assert(entity != entt::null);

                auto& trigger = registry->emplace<ContextualDialogTriggerComponent>(entity);
                std::vector<std::string> text;

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
                        else if (buff.find("distance: ") != std::string::npos)
                        {
                            auto sub = buff.substr(std::string("distance: ").size());
                            float dist = std::stof(sub);
                            trigger.distance = dist;
                        }
                        else if (buff.find("speaker: ") != std::string::npos)
                        {
                            auto sub = buff.substr(std::string("speaker: ").size());
                            entt::entity speaker = gameData->renderSystem->FindRenderable(sub);
                            assert(trigger.speaker != entt::null);
                            trigger.speaker = speaker;
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

    // Selected player triggers an overhead dialog to appear over the `speaker` (declared in the trigger's file)
    // based on `distance` (declared in the trigger's file).
    void ContextualDialogSystem::Update() const
    {
        for (const auto view = registry->view<sgTransform, ContextualDialogTriggerComponent, Renderable>();
             auto entity : view)
        {
            auto& trigger = registry->get<ContextualDialogTriggerComponent>(entity);
            const auto speaker = trigger.speaker;

            if (registry->any_of<OverheadDialogComponent>(speaker))
            {
                if (auto& overhead = registry->get<OverheadDialogComponent>(speaker); overhead.IsFinished())
                {
                    registry->erase<OverheadDialogComponent>(speaker);
                }
                else
                {
                    continue;
                }
            }

            const auto playerPos =
                registry->get<sgTransform>(gameData->controllableActorSystem->GetSelectedActor()).GetWorldPos();

            // As the contextual object could be a static mesh, using its collideable (which goes off the model's
            // vertex data) is a safer idea.
            const auto& col = registry->get<Collideable>(entity);
            auto center =
                Vector3MultiplyByValue(Vector3Add(col.worldBoundingBox.max, col.worldBoundingBox.min), 0.5f);

            if (trigger.CanTrigger() && Vector3Distance(center, playerPos) < trigger.distance)
            {
                auto& overhead = registry->emplace<OverheadDialogComponent>(speaker);
                const auto contextualDialog = dialogTextMap.at(entity);
                overhead.SetText(contextualDialog, 3.0f);
                trigger.SetTriggered();
            }
        }
    }

    ContextualDialogSystem::ContextualDialogSystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }

} // namespace sage