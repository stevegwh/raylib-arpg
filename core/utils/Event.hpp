//
// Created by steve on 05/01/2025.
//

#pragma once

#include <functional>
#include <unordered_map>

namespace sage
{
    using SubscriberId = unsigned int;

    template <typename... Args>
    class Event
    {
        unsigned int count = 0;
        std::unordered_map<SubscriberId, std::function<void(Args...)>> subscribers;

      public:
        SubscriberId Subscribe(std::function<void(Args...)> func)
        {
            subscribers.emplace(++count, func);
            return count;
        }

        void Publish(Args... args)
        {
            for (const auto& [key, subscriber] : subscribers)
            {
                subscriber(args...);
            }
        }

        void Unsubscribe(SubscriberId id)
        {
            subscribers.erase(id);
        }
    };

} // namespace sage
