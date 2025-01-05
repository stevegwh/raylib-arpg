//
// Created by Steve Wheeler on 29/11/2024.
//

#include "QuestManager.hpp"

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

namespace fs = std::filesystem;
static constexpr auto QUEST_PATH = "resources/quests";

namespace sage
{
    enum class EventType
    {
        OnStart,
        OnComplete
    };

    void QuestManager::bindFunctionToQuestEvent(
        const std::string& functionName, const std::string& functionParams, Quest* quest, EventType eventType)
    {
        // TODO:
        // As we have a separation between systems and entities, it's common that a system subscribes to an event
        // on behalf of an entity, but the event does not return back which entity is the one that wanted to react
        // to the event. This creates a big issue that could be solved nicely with captured lambdas, e.g.:
        // sink.connect<[&npcName](PartySystem& partySystem, entt::entity)
        // {partySystem.NPCToMember(npcName);}(*gameData->partySystem);
        // But, we are not allowed to capture anything
        // in an entt lamba, so we can't pass the string name as an argument in this way. Not sure how to
        // avoid this in a better way.
        // The current "solution" I have created for this is using "SignalHooks" (classes that subscribe on behalf
        // of the entity and forward on the entity's id). But its quite messy.

        // Just use std::function and capturing lambdas. Don't even really need a library, just a templated class
        // with a list of std::function

        if (functionName.find("OpenDoor") != std::string::npos)
        {
            auto startPos = functionParams.find_first_of('"');
            auto endPos = functionParams.find_last_of('"');
            std::string doorName = functionParams.substr(startPos + 1, endPos - (startPos + 1));

            auto doorId = gameData->renderSystem->FindRenderable<DoorBehaviorComponent>(doorName);
            assert(doorId != entt::null);

            if (eventType == EventType::OnStart)
            {
                quest->onQuestStart->Subscribe(
                    [doorId, this](entt::entity) { gameData->doorSystem->UnlockAndOpenDoor(doorId); });
            }
            else if (eventType == EventType::OnComplete)
            {
                quest->onQuestCompleted->Subscribe(
                    [doorId, this](entt::entity) { gameData->doorSystem->UnlockAndOpenDoor(doorId); });
            }
        }
        else if (functionName.find("JoinParty") != std::string::npos)
        {

            auto startPos = functionParams.find_first_of('"');
            auto endPos = functionParams.find_last_of('"');
            std::string npcName = functionParams.substr(startPos + 1, endPos - (startPos + 1));

            auto npcId = gameData->renderSystem->FindRenderable(npcName);

            if (eventType == EventType::OnStart)
            {
                quest->onQuestStart->Subscribe(
                    [npcId, this](entt::entity) { gameData->partySystem->NPCToMember(npcId); });
            }
            else if (eventType == EventType::OnComplete)
            {
                quest->onQuestCompleted->Subscribe(
                    [npcId, this](entt::entity) { gameData->partySystem->NPCToMember(npcId); });
            }
        }
        else
        {
            // assert(0);
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
                            if (taskLine.find("dialog: ") != std::string::npos)
                            {
                                auto sub = taskLine.substr(std::string("dialog: ").size());
                                auto command = sub.find_first_of(',');
                                if (command != std::string::npos)
                                {
                                    sub = sub.substr(0, command);
                                }
                                auto speakToEntity = gameData->renderSystem->FindRenderable(sub);
                                assert(speakToEntity != entt::null);
                                registry->emplace<QuestTaskComponent>(speakToEntity, questName);
                                quest.AddTask(speakToEntity);
                                assert(registry->any_of<DialogComponent>(speakToEntity));
                            }
                            else if (taskLine.find("item: ") != std::string::npos)
                            {
                                auto sub = taskLine.substr(std::string("item: ").size());
                                auto command = sub.find_first_of(',');
                                if (command != std::string::npos)
                                {
                                    sub = sub.substr(0, command);
                                }
                                auto pickupEntity = gameData->renderSystem->FindRenderable(sub);
                                assert(pickupEntity != entt::null);
                                registry->emplace<QuestTaskComponent>(pickupEntity, questName);
                                quest.AddTask(pickupEntity);
                                assert(registry->any_of<ItemComponent>(pickupEntity));
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
                            bindFunctionToQuestEvent(functionName, functionParams, &quest, EventType::OnStart);
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
                            bindFunctionToQuestEvent(functionName, functionParams, &quest, EventType::OnComplete);
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
        // if (!map.contains(key))
        // {
        //     return createQuest(key);
        // }
        return map.at(key);
    }

    // void QuestManager::AddTaskToQuest(const std::string& questKey, entt::entity taskId)
    // {
    //     auto& quest = registry->get<Quest>(GetQuest(questKey));
    //     quest.AddTask(taskId);
    // }
    //
    // void QuestManager::AddTaskToQuest(entt::entity questId, entt::entity taskId)
    // {
    //     auto& quest = registry->get<Quest>(questId);
    //     quest.AddTask(taskId);
    // }

    QuestManager::QuestManager(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }

} // namespace sage