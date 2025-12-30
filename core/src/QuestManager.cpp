//
// Created by Steve Wheeler on 29/11/2024.
//

#include "QuestManager.hpp"

#include "components/QuestComponents.hpp"
#include "GameUiEngine.hpp"
#include "TextToRealFunction.hpp"

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
                    else if (buff.find("<description>") != std::string::npos)
                    {
                        std::string description;
                        std::string descriptionLine;
                        while (std::getline(ss, descriptionLine) &&
                               descriptionLine.find("</description>") == std::string::npos)
                        {
                            description += descriptionLine + "\n";
                        }
                        quest.journalDescription = description;
                    }
                    else if (buff.find("<tasks>") != std::string::npos)
                    {
                        std::string taskLine;
                        while (std::getline(ss, taskLine) && taskLine.find("</tasks>") == std::string::npos)
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
                                entity = sys->renderSystem->FindRenderable(sub);
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
                                entity = sys->renderSystem->FindRenderable(sub);
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
                                    BindFunctionToEvent<Event<QuestTaskComponent*>, QuestTaskComponent*>(
                                        registry, sys, func, &task.onCompleted);
                                }
                            }
                        }
                    }
                    else if (buff.find("<onStart>") != std::string::npos)
                    {
                        std::string functionLine;
                        while (std::getline(ss, functionLine) &&
                               functionLine.find("</onStart>") == std::string::npos)
                        {
                            auto func = getFunctionNameAndArgs(functionLine);
                            BindFunctionToEvent<Event<entt::entity>, entt::entity>(
                                registry, sys, func, &quest.onStart);
                        }
                    }
                    else if (buff.find("<onComplete>") != std::string::npos)
                    {
                        std::string functionLine;
                        while (std::getline(ss, functionLine) &&
                               functionLine.find("</onComplete>") == std::string::npos)
                        {
                            auto func = getFunctionNameAndArgs(functionLine);
                            BindFunctionToEvent<Event<entt::entity>, entt::entity>(
                                registry, sys, func, &quest.onCompleted);
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

    void QuestManager::onComponentAdded(entt::entity entity)
    {
        auto& quest = registry->get<Quest>(entity);
        auto& vec = connectionMap[entity];
        vec.push_back(quest.onStart.Subscribe([this](entt::entity _entity) {
            onQuestUpdate.Publish(_entity);
            sys->uiEngine->CreateErrorMessage("Quest added to journal.");
        }));
        vec.push_back(
            quest.onCompleted.Subscribe([this](entt::entity _entity) { onQuestUpdate.Publish(_entity); }));
    }

    void QuestManager::onComponentRemoved(entt::entity entity)
    {
        auto& vec = connectionMap.at(entity);
        for (auto& sub : vec)
        {
            sub.UnSubscribe();
        }
    }

    void QuestManager::RemoveQuest(const std::string& key)
    {
        map.erase(key);
        // TODO: Surely remove from registry?
    }

    std::vector<Quest*> QuestManager::GetActiveQuests()
    {
        std::vector<Quest*> out;
        for (const auto& [key, value] : map)
        {
            auto& quest = registry->get<Quest>(value);
            if (quest.HasStarted() && !quest.IsComplete())
            {
                out.push_back(&quest);
            }
        }
        return out;
    }

    entt::entity QuestManager::GetQuest(const std::string& key) const
    {
        assert(map.contains(key));
        return map.at(key);
    }

    QuestManager::QuestManager(entt::registry* _registry, Systems* _sys) : registry(_registry), sys(_sys)
    {
        registry->on_construct<Quest>().connect<&QuestManager::onComponentAdded>(this);
        registry->on_destroy<Quest>().connect<&QuestManager::onComponentRemoved>(this);
    }

} // namespace sage