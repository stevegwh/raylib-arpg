//
// Created by Steve Wheeler on 29/11/2024.
//

#include "QuestManager.hpp"

#include "components/DialogComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/QuestComponents.hpp"
#include "GameData.hpp"
#include "systems/RenderSystem.hpp"

#include <cassert>
#include <filesystem>
#include <format>
#include <fstream>

namespace fs = std::filesystem;
static constexpr auto QUEST_PATH = "resources/quests";

namespace sage
{

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
                                auto speakToEntity = gameData->renderSystem->FindRenderable(sub);
                                assert(speakToEntity != entt::null);
                                registry->emplace<QuestTaskComponent>(speakToEntity, questName);
                                quest.AddTask(speakToEntity);
                                assert(registry->any_of<DialogComponent>(speakToEntity));
                            }
                            else if (taskLine.find("item: ") != std::string::npos)
                            {
                                auto sub = taskLine.substr(std::string("item: ").size());
                                auto pickupEntity = gameData->renderSystem->FindRenderable(sub);
                                assert(pickupEntity != entt::null);
                                registry->emplace<QuestTaskComponent>(pickupEntity, questName);
                                quest.AddTask(pickupEntity);
                                assert(registry->any_of<ItemComponent>(pickupEntity));
                            }
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