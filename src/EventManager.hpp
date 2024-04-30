//
// Created by Steve Wheeler on 28/03/2024.
//

#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <utility>

#include "Event.hpp"
#include "EventCallback.hpp"

namespace sage
{

class EventManager
{
public:
    std::vector<std::shared_ptr<EventCallback>> callbacks;
    void Subscribe(std::function<void()> func, sage::Event& event);
};

} // sage
