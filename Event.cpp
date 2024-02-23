//
// Created by Steve Wheeler on 29/09/2023.
//

#include "Event.hpp"

namespace sage
{

void Event::InvokeAllCallbacks() const
{
    for (const std::shared_ptr<EventCallback>& ob : callbacks)
    {
        ob->callback();
    }
}

void Event::Subscribe(const std::shared_ptr<EventCallback>& callback)
{
    callbacks.push_back(callback);
}

void Event::Unsubscribe(const std::shared_ptr<EventCallback>& callback)
{
    for (auto it = callbacks.begin(); it != callbacks.end(); ++it)
    {
        if (*it == callback)
        {
            callbacks.erase(it);
            break;
        }
    }
}

}