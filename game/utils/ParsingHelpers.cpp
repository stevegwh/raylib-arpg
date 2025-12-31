//
// Created by steve on 07/01/2025.
//

#include "ParsingHelpers.hpp"

#include "components/QuestComponents.hpp"
#include "QuestManager.hpp"
#include "Systems.hpp"
#include "systems/PartySystem.hpp"

#include "engine/components/Renderable.hpp"
#include "engine/systems/RenderSystem.hpp"

#include <regex>
#include <sstream>

namespace lq::parsing
{

    std::string removeCommentsFromFile(const std::string& fileContents)
    {
        std::stringstream ss(fileContents);
        std::string out;
        out.reserve(fileContents.length()); // Worth it? What if the comments take up a huge amount of space?
        std::string buff;
        while (std::getline(ss, buff, '\n'))
        {
            buff = trim(buff);
            if (buff.starts_with("//"))
            {
                continue;
            }
            auto commentPos = buff.find("//");
            if (commentPos != std::string::npos)
            {
                out.append(buff.substr(0, commentPos) + '\n');
            }
            else
            {
                out.append(buff + '\n');
            }
        }
        return out;
    }

    std::string trim(const std::string& str)
    {
        const auto start = str.find_first_not_of(" \t\n\r");
        const auto end = str.find_last_not_of(" \t\n\r");
        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    }

    std::string trimWhiteSpaceFromFile(const std::string& fileContents)
    {
        std::string out;
        out.reserve(fileContents.length());

        std::stringstream ss(fileContents);
        std::string buff;
        ;
        while (std::getline(ss, buff, '\n'))
        {
            out.append(trim(buff) + "\n");
        }
        return out;
    }

    std::string normalizeLineEndings(const std::string& content)
    {
        std::string normalized = content;
        normalized = std::regex_replace(normalized, std::regex("\r\n"), "\n");
        normalized = std::regex_replace(normalized, std::regex("\r"), "\n");
        return normalized;
    }

    TextFunction getFunctionNameAndArgs(const std::string& input)
    {
        std::string trimmedInput = trim(input);

        std::regex pattern(R"(^(\w+)\(([^)]*)\)$)");
        std::smatch match;

        if (std::regex_match(trimmedInput, match, pattern))
        {
            std::string functionName = match[1];
            std::string parameter = match[2];

            return {functionName, parameter};
        }
        else
        {
            return {trimmedInput, ""};
        }
    }

    std::function<bool()> GetConditionalStatement(const std::string& line, entt::registry* registry, Systems* sys)
    {
        return [registry, sys, line]() -> bool {
            auto quest_complete = [&](const std::string& params) -> bool {
                const auto& quest = registry->get<Quest>(sys->questManager->GetQuest(params));
                return quest.IsComplete();
            };

            auto quest_in_progress = [&](const std::string& params) -> bool {
                const auto& quest = registry->get<Quest>(sys->questManager->GetQuest(params));
                return quest.HasStarted() && !quest.IsComplete();
            };

            auto has_item = [&](const std::string& params) -> bool {
                return sys->partySystem->CheckPartyHasItem(params);
            };

            auto quest_all_tasks_complete = [&](const std::string& params) -> bool {
                const auto& quest = registry->get<Quest>(sys->questManager->GetQuest(params));
                return quest.HasStarted() && quest.AllTasksComplete();
            };

            auto quest_task_complete = [&](const std::string& params) -> bool {
                const auto entity = sys->renderSystem->FindRenderable(params);
                assert(entity != entt::null);
                const auto& questComponent = registry->get<QuestTaskComponent>(entity);
                return questComponent.IsComplete();
            };

            const std::unordered_map<std::string, std::function<bool(std::string)>> functionMap = {
                {"quest_complete", quest_complete},
                {"quest_in_progress", quest_in_progress},
                {"has_item", has_item},
                {"quest_all_tasks_complete", quest_all_tasks_complete},
                {"quest_task_complete", quest_task_complete}};

            bool out = false;
            bool positive = true;
            bool andCondition = false;
            bool orCondition = false;
            bool isFirstCondition = true;

            std::stringstream condStream(trim(line.substr(line.find("if") + 2)));
            std::string current;

            while (std::getline(condStream, current, ' '))
            {
                if (current == "not")
                {
                    positive = false;
                    continue;
                }
                if (current == "and")
                {
                    andCondition = true;
                    continue;
                }
                if (current == "or")
                {
                    orCondition = true;
                    continue;
                }
                auto func = getFunctionNameAndArgs(current);

                assert(!func.name.empty());
                assert(!func.params.empty());
                assert(functionMap.contains(func.name));

                auto funcResult = functionMap.at(func.name)(func.params);
                const bool currentResult = positive == funcResult; // 'not'

                if (isFirstCondition)
                {
                    isFirstCondition = false;
                    out = currentResult;
                }
                else if (andCondition)
                {
                    out = out && currentResult;
                }
                else if (orCondition)
                {
                    out = out || currentResult;
                }

                positive = true;
                andCondition = false;
                orCondition = false;
            }
            return out;
        };
    }
} // namespace lq::parsing
