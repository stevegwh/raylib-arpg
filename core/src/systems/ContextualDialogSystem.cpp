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
#include "Settings.hpp"
#include "slib.hpp"
#include "Systems.hpp"

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

                std::vector<std::string> text;
                entt::entity entity = entt::null;

                if (file.is_open())
                {
                    ContextualDialogTriggerComponent* trigger = nullptr;

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
                        if (buff.find("owner: ") != std::string::npos)
                        {
                            auto sub = buff.substr(std::string("owner: ").size());
                            entity = sys->renderSystem->FindRenderable(sub);
                            trigger = &registry->emplace<ContextualDialogTriggerComponent>(entity);
                            assert(entity != entt::null && trigger);
                        }
                        else if (buff.find("distance: ") != std::string::npos)
                        {
                            assert(entity != entt::null && trigger);
                            auto sub = buff.substr(std::string("distance: ").size());
                            float dist = std::stof(sub);
                            trigger->distance = dist;
                        }
                        else if (buff.find("speaker: ") != std::string::npos)
                        {
                            assert(entity != entt::null && trigger);
                            auto sub = buff.substr(std::string("speaker: ").size());
                            entt::entity speaker = sys->renderSystem->FindRenderable(sub);
                            assert(trigger->speaker != entt::null);
                            trigger->speaker = speaker;
                        }
                        else if (buff.find("loop: ") != std::string::npos)
                        {
                            assert(entity != entt::null && trigger);
                            auto sub = buff.substr(std::string("loop: ").size());
                            if (sub.find("true") != std::string::npos)
                            {
                                trigger->loop = true;
                            }
                        }
                        else if (buff.find("should_retrigger: ") != std::string::npos)
                        {
                            assert(entity != entt::null && trigger);
                            auto sub = buff.substr(std::string("should_retrigger: ").size());
                            if (sub.find("true") != std::string::npos)
                            {
                                trigger->shouldRetrigger = true;
                            }
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

    // Selected player triggers an overhead dialog to appear over the `speaker` (declared in the trigger's file)
    // based on `distance` (declared in the trigger's file).
    void ContextualDialogSystem::Update() const
    {
        for (const auto view = registry->view<sgTransform, ContextualDialogTriggerComponent, Renderable>();
             auto entity : view)
        {
            auto& trigger = registry->get<ContextualDialogTriggerComponent>(entity);
            const auto speaker = trigger.speaker;
            const auto playerPos =
                registry->get<sgTransform>(sys->controllableActorSystem->GetSelectedActor()).GetWorldPos();

            // As the contextual object could be a static mesh, using its collideable (which goes off the model's
            // vertex data) is a safer idea.
            const auto& col = registry->get<Collideable>(entity);
            auto center =
                Vector3MultiplyByValue(Vector3Add(col.worldBoundingBox.max, col.worldBoundingBox.min), 0.5f);

            if (!trigger.HasTriggered())
            {
                if (registry->any_of<OverheadDialogComponent>(speaker)) continue;
                if (Vector3Distance(center, playerPos) < trigger.distance)
                {
                    auto& overhead =
                        registry->emplace_or_replace<OverheadDialogComponent>(speaker, trigger.ShouldLoop());
                    const auto contextualDialog = dialogTextMap.at(entity);
                    overhead.SetText(contextualDialog, 5.0f);
                    trigger.SetTriggered();
                }
            }
            else
            {
                if (!registry->any_of<OverheadDialogComponent>(speaker)) continue;
                if (auto& overhead = registry->get<OverheadDialogComponent>(speaker);
                    overhead.IsFinished() && Vector3Distance(center, playerPos) > trigger.distance)
                {
                    registry->erase<OverheadDialogComponent>(speaker);
                    trigger.SetTriggered(!trigger.shouldRetrigger);
                }
            }
        }
    }

    void ContextualDialogSystem::Draw2D() const
    {
        for (const auto view = registry->view<OverheadDialogComponent>(); const auto& entity : view)
        {
            const auto& col = registry->get<Collideable>(entity);
            auto [width, height] = sys->settings->GetViewPort();
            auto center =
                Vector3MultiplyByValue(Vector3Add(col.worldBoundingBox.max, col.worldBoundingBox.min), 0.5f);
            const auto pos = Vector3{center.x, col.worldBoundingBox.max.y + 1.0f, center.z};
            auto screenPos = GetWorldToScreenEx(pos, *sys->camera->getRaylibCam(), width, height);
            auto& contextualDiag = registry->get<OverheadDialogComponent>(entity);

            auto scaledFontSize = sys->settings->ScaleValueMaintainRatio(fontSize);
            const auto text = contextualDiag.GetText();
            DrawText(
                text.c_str(),
                static_cast<float>(static_cast<int>(screenPos.x) - MeasureText(text.c_str(), scaledFontSize) / 2),
                static_cast<float>(static_cast<int>(screenPos.y)),
                scaledFontSize,
                WHITE);
        }
    }

    ContextualDialogSystem::ContextualDialogSystem(entt::registry* _registry, Systems* _sys)
        : registry(_registry), sys(_sys)
    {
    }

} // namespace sage