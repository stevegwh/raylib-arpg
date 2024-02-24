//
// Created by Steve Wheeler on 29/09/2023.
//

#pragma once

#include <functional>
#include <vector>
#include <memory>

#include "EventCallback.hpp"

namespace sage
{

class Event
{
    private:
        std::vector<std::shared_ptr<EventCallback>> callbacks{};
    
    public:
        // Shared pointers (ownership) so I can unsubscribe using an EventCallback stored elsewhere.
        void Subscribe(const std::shared_ptr<EventCallback>& callback);
        void Unsubscribe(const std::shared_ptr<EventCallback>& callback);
        void InvokeAllCallbacks() const;
    };
}
