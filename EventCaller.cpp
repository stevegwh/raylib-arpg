//
// Created by Steve Wheeler on 28/03/2024.
//

#include "EventCaller.hpp"

namespace sage
{
void EventCaller::Subscribe(std::function<void()> func, Event& event)
{
    auto e1 = std::make_shared<EventCallback>(func);
    eventCallbacks.push_back(e1);
    event.Subscribe(e1);
}
}