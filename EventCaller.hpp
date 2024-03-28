//
// Created by Steve Wheeler on 28/03/2024.
//

#pragma once

#include <functional>
#include <vector>
#include <memory>

#include "Event.hpp"
#include "EventCallback.hpp"

namespace sage
{
class EventCaller
{
public:
    std::vector<std::shared_ptr<EventCallback>> eventCallbacks;
    void Subscribe(std::function<void()> func, Event& event);
};

} // sage
