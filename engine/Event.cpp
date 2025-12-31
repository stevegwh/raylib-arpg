//
// Created by steve on 05/01/2025.
//

#include "Event.hpp"

#include <iostream>

namespace sage
{
    bool Subscription::IsActive()
    {
        return id > -1 && event;
    }

    void Subscription::UnSubscribe()
    {
        if (!IsActive()) return;
        event->unSubscribe(id);
        id = -1;
        event = nullptr;
    }

    Subscription::~Subscription()
    {
        // UnSubscribe();
    }

    Subscription::Subscription(EventBase* _event, SubscriberId _id) : event(_event), id(_id)
    {
    }
} // namespace sage