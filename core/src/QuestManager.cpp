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
#include "systems/DoorSystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/RenderSystem.hpp"

#include <cassert>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;
static constexpr auto QUEST_PATH = "resources/quests";

namespace sage
{

    template <typename QuestEvent, typename... Args>
    void bindFunctionToEvent(
        entt::registry* registry,
        GameData* gameData,
        const std::string& functionName,
        const std::string& functionParams,
        QuestEvent* event)
    {
        if (functionName.find("OpenDoor") != std::string::npos)
        {
            auto startPos = functionParams.find_first_of('"');
            auto endPos = functionParams.find_last_of('"');
            std::string doorName = functionParams.substr(startPos + 1, endPos - (startPos + 1));

            auto doorId = gameData->renderSystem->FindRenderable<DoorBehaviorComponent>(doorName);
            assert(doorId != entt::null);

            event->Subscribe([doorId, gameData](Args...) { gameData->doorSystem->UnlockAndOpenDoor(doorId); });
        }
        else if (functionName.find("JoinParty") != std::string::npos)
        {

            auto startPos = functionParams.find_first_of('"');
            auto endPos = functionParams.find_last_of('"');
            std::string npcName = functionParams.substr(startPos + 1, endPos - (startPos + 1));

            auto npcId = gameData->renderSystem->FindRenderable(npcName);

            event->Subscribe([npcId, gameData](Args...) { gameData->partySystem->NPCToMember(npcId); });
        }
        else if (functionName.find("RemoveItem") != std::string::npos)
        {

            auto startPos = functionParams.find_first_of('"');
            auto endPos = functionParams.find_last_of('"');
            std::string itemName = functionParams.substr(startPos + 1, endPos - (startPos + 1));

            auto itemId = gameData->renderSystem->FindRenderable(itemName);

            event->Subscribe([itemId, gameData](Args...) { gameData->partySystem->RemoveItemFromParty(itemId); });
        }
        else if (functionName.find("GiveItem") != std::string::npos)
        {
            auto startPos = functionParams.find_first_of('"');
            auto endPos = functionParams.find_last_of('"');
            std::string itemName = functionParams.substr(startPos + 1, endPos - (startPos + 1));

            event->Subscribe(
                [itemName, gameData](Args...) { gameData->partySystem->GiveItemToSelected(itemName); });
        }
        else if (functionName.find("PlaySFX") != std::string::npos)
        {

            auto startPos = functionParams.find_first_of('"');
            auto endPos = functionParams.find_last_of('"');
            std::string sfxName = functionParams.substr(startPos + 1, endPos - (startPos + 1));

            event->Subscribe([sfxName, gameData](Args...) { gameData->audioManager->PlaySFX(sfxName); });
        }
        else if (functionName.find("DisableWorldItem") != std::string::npos)
        {
            auto startPos = functionParams.find_first_of('"');
            auto endPos = functionParams.find_last_of('"');
            std::string worldItemName = functionParams.substr(startPos + 1, endPos - (startPos + 1));
            auto itemId = gameData->renderSystem->FindRenderable(worldItemName);
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
        else if (functionName.find("EndGame") != std::string::npos)
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
                std::string fileName = entry.path().filename().string();
                std::ifstream file{std::format("{}/{}", QUEST_PATH, fileName)};
                std::string questName = StripPath(fileName);

                auto& quest = registry->get<Quest>(createQuest(questName));

                // TODO: Must make sure renderable and item systems are intialised before this
                if (!file.is_open()) return;

                std::string buff;
                while (std::getline(file, buff, '\n'))
                {
                    if (buff.find("[Meta]") != std::string::npos)
                    {
                        std::string metaLine;
                        while (std::getline(file, metaLine) && !metaLine.empty())
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
                        while (std::getline(file, descriptionLine) && !descriptionLine.empty())
                        {
                            description += descriptionLine + "\n";
                        }
                        quest.journalDescription = description;
                    }
                    else if (buff.find("[Tasks]") != std::string::npos)
                    {
                        std::string taskLine;
                        while (std::getline(file, taskLine) && !taskLine.empty())
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
                                    remove_if(commandLine.begin(), commandLine.end(), isspace), commandLine.end());
                                std::stringstream commandStream(commandLine);
                                std::string command;

                                while (std::getline(commandStream, command, ';'))
                                {
                                    auto paramStartPos = command.find_first_of('(');
                                    auto paramEndPos = command.find_last_of(')');
                                    auto functionName = command.substr(0, paramStartPos);
                                    auto functionParams =
                                        command.substr(paramStartPos + 1, paramEndPos - (paramStartPos + 1));
                                    bindFunctionToEvent<Event<QuestTaskComponent*>, QuestTaskComponent*>(
                                        registry, gameData, functionName, functionParams, task.onCompleted.get());
                                }
                            }
                        }
                    }
                    else if (buff.find("[OnStart]") != std::string::npos)
                    {
                        std::string functionLine;
                        while (std::getline(file, functionLine) && !functionLine.empty())
                        {
                            auto paramStartPos = functionLine.find_first_of('(');
                            auto paramEndPos = functionLine.find_last_of(')');
                            auto functionName = functionLine.substr(0, paramStartPos);
                            auto functionParams =
                                functionLine.substr(paramStartPos + 1, paramEndPos - (paramStartPos + 1));
                            bindFunctionToEvent<Event<entt::entity>, entt::entity>(
                                registry, gameData, functionName, functionParams, quest.onStart.get());
                        }
                    }
                    else if (buff.find("[OnComplete]") != std::string::npos)
                    {
                        std::string functionLine;
                        while (std::getline(file, functionLine) && !functionLine.empty())
                        {
                            auto paramStartPos = functionLine.find_first_of('(');
                            auto paramEndPos = functionLine.find_last_of(')');
                            auto functionName = functionLine.substr(0, paramStartPos);
                            auto functionParams =
                                functionLine.substr(paramStartPos + 1, paramEndPos - (paramStartPos + 1));
                            bindFunctionToEvent<Event<entt::entity>, entt::entity>(
                                registry, gameData, functionName, functionParams, quest.onCompleted.get());
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

    entt::entity QuestManager::GetQuest(const std::string& key)
    {
        assert(map.contains(key));
        return map.at(key);
    }

    QuestManager::QuestManager(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }

} // namespace sage