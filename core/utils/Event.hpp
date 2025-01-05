//
// Created by steve on 05/01/2025.
//

#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

namespace sage
{

    using SubscriberId = int;

    template <typename... Args>
    class Event;

    class BaseConnection
    {
      public:
        virtual void UnSubscribe() = 0;
        virtual ~BaseConnection() = default;
    };

    template <typename... Args>
    class Connection : public BaseConnection
    {
        Event<Args...>* event;
        SubscriberId id = -1;

      public:
        void UnSubscribe() override
        {
            event->unSubscribe(id);
        }

        explicit Connection(Event<Args...>* _event, const SubscriberId _id) : event(_event), id(_id)
        {
        }

        Connection() = default;
    };

    template <typename... Args>
    class Event
    {
        int count = 0;
        std::unordered_map<SubscriberId, std::function<void(Args...)>> subscribers;

        void unSubscribe(SubscriberId& id)
        {
            if (id == -1) return;
            subscribers.erase(id);
            id = -1;
        }

      public:
        // [[nodiscard]]
        std::shared_ptr<Connection<Args...>> Subscribe(std::function<void(Args...)> func)
        {
            subscribers.emplace(++count, func);

            return std::make_shared<Connection<Args...>>(this, count);
        }

        void Publish(Args... args)
        {
            for (const auto& [key, subscriber] : subscribers)
            {
                subscriber(args...);
            }
        }

        friend class Connection<Args...>;
    };

} // namespace sage
