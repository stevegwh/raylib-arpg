//
// Created by Steve Wheeler on 29/09/2023.
//

#pragma once

#include <functional>
#include <unordered_map>
#include <memory>

#include "EventCallback.hpp"

namespace sage
{

class Event
{
    private:
        std::unordered_map<std::string, std::weak_ptr<EventCallback>> callbacks{};
    
    public:
        void Subscribe(const std::shared_ptr<EventCallback>& callback);
        void Unsubscribe(const std::string& signature);
        void InvokeAllCallbacks();
    };
}
