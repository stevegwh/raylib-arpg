//
// Created by Steve Wheeler on 29/09/2023.
//

#include "Event.hpp"

namespace sage
{

void Event::InvokeAllCallbacks()  
{
    for (auto it = callbacks.begin(); it != callbacks.end();) 
    {
        auto callback = it->second.lock();
        if (!callback) 
        {
            it = callbacks.erase(it);
        } 
        else 
        {
            callback->callback();
            ++it;
        }
    }
}

void Event::Unsubscribe(const std::string& signature)
{
    callbacks.erase(signature);
}

void Event::Subscribe(const std::shared_ptr<EventCallback>& callback)
{
    callbacks[callback->signature] = callback;
}

}