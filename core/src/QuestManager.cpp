//
// Created by Steve Wheeler on 29/11/2024.
//

#include "QuestManager.hpp"

#include "AudioManager.hpp"
#include "components/Collideable.hpp"
#include "components/DialogComponent.hpp"
#include "components/DoorBehaviorComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/QuestComponents.hpp"
#include "GameData.hpp"
#include "ParsingHelpers.hpp"
#include "systems/DoorSystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/RenderSystem.hpp"

#include <cassert>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <sstream>

namespace fs = std::filesystem;
static constexpr auto QUEST_PATH = "resources/quests";

namespace sage
{
    using namespace parsing;

    template <typename QuestEvent, typename... Args>
    void bindFunctionToEvent(entt::registry* registry, GameData* gameData, TextFunction func, QuestEvent* event)
    {
        assert(!func.name.empty());
        // Not all functions require params

        if (func.name.find("OpenDoor") != std::string::npos)
        {
            assert(!func.params.empty());
            auto doorId = gameData->renderSystem->FindRenderable<DoorBehaviorComponent>(func.params);
            assert(doorId != entt::null);
            event->Subscribe([doorId, gameData](Args...) { gameData->doorSystem->UnlockAndOpenDoor(doorId); });
        }
        else if (func.name.find("JoinParty") != std::string::npos)
        {
            assert(!func.params.empty());
            auto npcId = gameData->renderSystem->FindRenderable(func.params);
            assert(npcId != entt::null);
            event->Subscribe([npcId, gameData](Args...) { gameData->partySystem->NPCToMember(npcId); });
        }
        else if (func.name.find("RemoveItem") != std::string::npos)
        {
            assert(!func.params.empty());
            auto itemId = gameData->renderSystem->FindRenderable(func.params);
            assert(itemId != entt::null);
            event->Subscribe([itemId, gameData](Args...) { gameData->partySystem->RemoveItemFromParty(itemId); });
        }
        else if (func.name.find("GiveItem") != std::string::npos)
        {
            assert(!func.params.empty());
            event->Subscribe([itemName = func.params, gameData](Args...) {
                gameData->partySystem->GiveItemToSelected(itemName);
            });
        }
        else if (func.name.find("PlaySFX") != std::string::npos)
        {
            assert(!func.params.empty());
            event->Subscribe(
                [sfxName = func.params, gameData](Args...) { gameData->audioManager->PlaySFX(sfxName); });
        }
        else if (func.name.find("DisableWorldItem") != std::string::npos)
        {
            assert(!func.params.empty());
            auto itemId = gameData->renderSystem->FindRenderable(func.params);
            assert(itemId != entt::null);
            event->Subscribe([itemId, registry](Args...) {
                if (registry->any_of<Renderable>(itemId))
                {
                    registry->get<Renderable>(itemId).Disable();
                }
                if (registry->any_of<Collideable>(itemId))
                {
                    registry->get<Collideable>(itemId).Disable();
                }
            });
        }
        else if (func.name.find("EndGame") != std::string::npos)
        {
            // TODO
        }
        else
        {
            assert(0);
        }
    }

    void QuestManager::InitQuestsFromDirectory()
    {
        for (const fs::path path{QUEST_PATH}; const auto& entry : fs::directory_iterator(path))
        {
            if (entry.path().extension() == ".txt")
            {
                // TODO: Must make sure renderable and item systems are initialised before this
                std::string fileName = entry.path().filename().string();
                std::string questName = StripPath(fileName);
                std::ostringstream fileContent;

                // Scoped to prevent accidentally using 'file' instead of 'stringstream'
                {
                    std::ifstream file{std::format("{}/{}", QUEST_PATH, fileName)};
                    if (!file.is_open()) return;
                    fileContent << file.rdbuf();
                }

                auto& quest = registry->get<Quest>(createQuest(questName));
                std::string processed = trimAll(normalizeLineEndings(fileContent.str()));

                std::stringstream ss(processed);
                std::string buff;

                while (std::getline(ss, buff, '\n'))
                {
                    if (buff.find("[Meta]") != std::string::npos)
                    {
                        std::string metaLine;
                        while (std::getline(ss, metaLine) && !metaLine.empty())
                        {
                            if (metaLine.find("title: ") != std::string::npos)
                            {
                                auto sub = metaLine.substr(std::string("title: ").size());
                                quest.journalTitle = sub;
                            }
                            else if (metaLine.find("giver: ") != std::string::npos)
                            {
                                auto sub = metaLine.substr(std::string("giver: ").size());
                                // Do something here
                            }
                        }
                    }
                    else if (buff.find("[Description]") != std::string::npos)
                    {
                        std::string description;
                        std::string descriptionLine;
                        while (std::getline(ss, descriptionLine) && !descriptionLine.empty())
                        {
                            description += descriptionLine + "\n";
                        }
                        quest.journalDescription = description;
                    }
                    else if (buff.find("[Tasks]") != std::string::npos)
                    {
                        std::string taskLine;
                        while (std::getline(ss, taskLine) && !taskLine.empty())
                        {
                            std::string sub;
                            entt::entity entity;

                            auto commandStartPos = taskLine.find_first_of(';');

                            if (taskLine.find("dialog: ") != std::string::npos)
                            {
                                auto start = std::string("dialog: ").size();
                                if (commandStartPos != std::string::npos)
                                {
                                    sub = taskLine.substr(start, commandStartPos - start);
                                }
                                else
                                {
                                    sub = taskLine.substr(start);
                                }
                                entity = gameData->renderSystem->FindRenderable(sub);
                                assert(entity != entt::null);
                                assert(registry->any_of<DialogComponent>(entity));
                            }
                            else if (taskLine.find("item: ") != std::string::npos)
                            {
                                auto start = std::string("item: ").size();
                                if (commandStartPos != std::string::npos)
                                {
                                    sub = taskLine.substr(start, commandStartPos - start);
                                }
                                else
                                {
                                    sub = taskLine.substr(start);
                                }
                                entity = gameData->renderSystem->FindRenderable(sub);
                                assert(entity != entt::null);
                                assert(registry->any_of<ItemComponent>(entity));
                            }

                            auto& task = registry->emplace<QuestTaskComponent>(entity, questName);
                            quest.AddTask(entity);

                            if (commandStartPos != std::string::npos)
                            {
                                std::string commandLine = taskLine.substr(commandStartPos + 1);
                                commandLine.erase(
                                    std::ranges::remove_if(commandLine, isspace).begin(), commandLine.end());
                                std::stringstream commandStream(commandLine);
                                std::string command;

                                while (std::getline(commandStream, command, ';'))
                                {
                                    auto func = getFunctionNameAndArgs(command);
                                    bindFunctionToEvent<Event<QuestTaskComponent*>, QuestTaskComponent*>(
                                        registry, gameData, func, &task.onCompleted);
                                }
                            }
                        }
                    }
                    else if (buff.find("[OnStart]") != std::string::npos)
                    {
                        std::string functionLine;
                        while (std::getline(ss, functionLine) && !functionLine.empty())
                        {
                            auto func = getFunctionNameAndArgs(functionLine);
                            bindFunctionToEvent<Event<entt::entity>, entt::entity>(
                                registry, gameData, func, &quest.onStart);
                        }
                    }
                    else if (buff.find("[OnComplete]") != std::string::npos)
                    {
                        std::string functionLine;
                        while (std::getline(ss, functionLine) && !functionLine.empty())
                        {
                            auto func = getFunctionNameAndArgs(functionLine);
                            bindFunctionToEvent<Event<entt::entity>, entt::entity>(
                                registry, gameData, func, &quest.onCompleted);
                        }
                    }
                }
            }
        }
    }

    entt::entity QuestManager::createQuest(const std::string& key)
    {
        auto entity = registry->create();
        registry->emplace<Quest>(entity, registry, entity, key);
        map.emplace(key, entity);

        return entity;
    }

    void QuestManager::RemoveQuest(const std::string& key)
    {
        map.erase(key);
        // TODO: Surely remove from registry?
    }

    entt::entity QuestManager::GetQuest(const std::string& key) const
    {
        assert(map.contains(key));
        return map.at(key);
    }

    QuestManager::QuestManager(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }

} // namespace sage