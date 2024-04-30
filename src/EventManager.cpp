//
// Created by Steve Wheeler on 28/03/2024.
//

#include "EventManager.hpp"

namespace sage
{
void EventManager::Subscribe(std::function<void()> func, Event& event)
{
    auto e1 = std::make_shared<EventCallback>(func);
    callbacks.push_back(e1);
    event.Subscribe(e1);
}
}