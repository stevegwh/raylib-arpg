//
// Created by Steve Wheeler on 03/01/2025.
//

#include "ContextualDialogSystem.hpp"

#include "../TextToRealFunction.hpp"
#include "ControllableActorSystem.hpp"
#include "engine/AudioManager.hpp"
#include "engine/Camera.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/OverheadDialogComponent.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/GameUiEngine.hpp"
#include "engine/Settings.hpp"
#include "engine/slib.hpp"
#include "engine/systems/RenderSystem.hpp"

#include "components/ContextualDialogTriggerComponent.hpp"
#include "ParsingHelpers.hpp"
#include "Systems.hpp"

#include "engine/Cursor.hpp"
#include "raylib.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <utility>

namespace fs = std::filesystem;
constexpr auto CONTEXTUAL_DIALOG_PATH = "resources/dialog/contextual";

namespace lq
{
    using namespace parsing;

    void ContextualDialogSystem::InitContextualDialogsFromDirectory()
    {
        for (const fs::path path{CONTEXTUAL_DIALOG_PATH}; const auto& entry : fs::directory_iterator(path))
        {
            if (entry.path().extension() == ".txt")
            {

                std::ostringstream fileContent;
                {
                    std::string fileName = entry.path().filename().string();
                    std::ifstream file{std::format("{}/{}", CONTEXTUAL_DIALOG_PATH, fileName)};
                    if (!file.is_open()) assert(0);
                    if (!file.is_open()) return;
                    fileContent << file.rdbuf();
                }

                std::vector<std::pair<std::string, std::function<bool()>>> text;
                entt::entity entity = entt::null;
                ContextualDialogTriggerComponent* trigger = nullptr;

                std::string processed =
                    removeCommentsFromFile(trimWhiteSpaceFromFile(normalizeLineEndings(fileContent.str())));
                std::stringstream ss(processed);
                std::string buff;
                while (std::getline(ss, buff, '\n'))
                {
                    if (buff.find("<meta>") != std::string::npos)
                    {
                        std::string metaLine;
                        while (std::getline(ss, metaLine) && metaLine.find("</meta>") == std::string::npos)
                        {
                            if (metaLine.find("owner: ") != std::string::npos)
                            {
                                auto sub = metaLine.substr(std::string("owner: ").size());
                                entity = sys->renderSystem->FindRenderable(sub);
                                trigger = &registry->emplace<ContextualDialogTriggerComponent>(entity);
                                assert(entity != entt::null && trigger);
                            }
                            else if (metaLine.find("distance: ") != std::string::npos)
                            {
                                assert(entity != entt::null && trigger);
                                auto sub = metaLine.substr(std::string("distance: ").size());
                                float dist = std::stof(sub);
                                trigger->distance = dist;
                            }
                            else if (metaLine.find("speaker: ") != std::string::npos)
                            {
                                assert(entity != entt::null && trigger);
                                auto sub = metaLine.substr(std::string("speaker: ").size());
                                entt::entity speaker = sys->renderSystem->FindRenderable(sub);
                                assert(trigger->speaker != entt::null);
                                trigger->speaker = speaker;
                            }
                            else if (metaLine.find("loop: ") != std::string::npos)
                            {
                                assert(entity != entt::null && trigger);
                                auto sub = metaLine.substr(std::string("loop: ").size());
                                if (sub.find("true") != std::string::npos)
                                {
                                    trigger->loop = true;
                                }
                            }
                            else if (metaLine.find("should_retrigger: ") != std::string::npos)
                            {
                                assert(entity != entt::null && trigger);
                                auto sub = metaLine.substr(std::string("should_retrigger: ").size());
                                if (sub.find("true") != std::string::npos)
                                {
                                    trigger->shouldRetrigger = true;
                                }
                            }
                        }
                    }
                    else if (buff.find("<dialog>") != std::string::npos)
                    {
                        std::optional<std::function<bool()>> condition;
                        std::string dialogLine;
                        while (std::getline(ss, dialogLine) && dialogLine.find("</dialog>") == std::string::npos)
                        {

                            if (dialogLine.starts_with("if"))
                            {
                                assert(!condition.has_value()); // "if blocks" must be closed with end. No nesting
                                                                // allowed (yet).
                                condition = GetConditionalStatement(dialogLine, registry, sys);
                            }
                            else if (dialogLine.starts_with("end"))
                            {
                                condition.reset();
                            }
                            else
                            {
                                if (condition.has_value())
                                {
                                    text.emplace_back(dialogLine, condition.value());
                                }
                                else
                                {
                                    text.emplace_back(dialogLine, []() { return true; });
                                }
                            }
                        }
                    }
                    else if (buff.find("<onTrigger>") != std::string::npos)
                    {
                        std::string commandLine;
                        while (std::getline(ss, commandLine) &&
                               commandLine.find("</onTrigger>") == std::string::npos)
                        {
                            auto func = getFunctionNameAndArgs(commandLine);
                            parsing::BindFunctionToEvent<sage::Event<>>(registry, sys, func, &trigger->onTrigger);
                        }
                    }
                }

                dialogTextMap.emplace(entity, text);
            }
        }
    }

    // Selected player triggers an overhead dialog to appear over the `speaker` (declared in the trigger's file)
    // based on `distance` (declared in the trigger's file).
    void ContextualDialogSystem::Update() const
    {
        for (const auto view =
                 registry->view<sage::sgTransform, ContextualDialogTriggerComponent, sage::Renderable>();
             auto entity : view)
        {
            auto& trigger = registry->get<ContextualDialogTriggerComponent>(entity);
            const auto speaker = trigger.speaker;
            const auto playerPos = registry->get<sage::sgTransform>(sys->cursor->GetSelectedActor()).GetWorldPos();

            // As the contextual object could be a static mesh, using its collideable (which goes off the model's
            // vertex data) is a safer idea.
            const auto& col = registry->get<sage::Collideable>(entity);
            auto center =
                sage::Vector3MultiplyByValue(Vector3Add(col.worldBoundingBox.max, col.worldBoundingBox.min), 0.5f);

            if (!trigger.HasTriggered())
            {
                if (registry->any_of<sage::OverheadDialogComponent>(speaker)) continue;
                if (Vector3Distance(center, playerPos) < trigger.distance)
                {
                    auto& overhead =
                        registry->emplace_or_replace<sage::OverheadDialogComponent>(speaker, trigger.ShouldLoop());
                    const auto contextualDialog = dialogTextMap.at(entity);
                    overhead.SetText(contextualDialog, 5.0f);
                    trigger.SetTriggered();
                }
            }
            else
            {
                if (!registry->any_of<sage::OverheadDialogComponent>(speaker)) continue;
                if (auto& overhead = registry->get<sage::OverheadDialogComponent>(speaker);
                    overhead.IsFinished() && Vector3Distance(center, playerPos) > trigger.distance)
                {
                    registry->erase<sage::OverheadDialogComponent>(speaker);
                    trigger.SetTriggered(!trigger.shouldRetrigger);
                }
            }
        }
    }

    void ContextualDialogSystem::Draw2D() const
    {
        for (const auto view = registry->view<sage::OverheadDialogComponent>(); const auto& entity : view)
        {
            const auto& col = registry->get<sage::Collideable>(entity);
            auto [width, height] = sys->settings->GetViewPort();
            auto center =
                sage::Vector3MultiplyByValue(Vector3Add(col.worldBoundingBox.max, col.worldBoundingBox.min), 0.5f);
            const auto pos = Vector3{center.x, col.worldBoundingBox.max.y + 1.0f, center.z};
            auto screenPos = GetWorldToScreenEx(pos, *sys->camera->getRaylibCam(), width, height);
            auto& contextualDiag = registry->get<sage::OverheadDialogComponent>(entity);

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

} // namespace lq