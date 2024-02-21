//
// Created by Steve Wheeler on 29/09/2023.
//

#include "Event.hpp"

namespace sage
{

void Event::InvokeAllCallbacks() const
{
    for (const std::shared_ptr<Observer>& ob : observerList)
    {
        ob->callback();
    }
}

void Event::Subscribe(const std::shared_ptr<Observer>& observer)
{
    observerList.push_back(observer);
}

void Event::Unsubscribe(const std::shared_ptr<Observer>& observer)
{
    for (auto it = observerList.begin(); it != observerList.end(); ++it)
    {
        if (*it == observer)
        {
            observerList.erase(it);
            break;
        }
    }
}

}