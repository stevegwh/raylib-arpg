//
// Created by steve on 05/01/2025.
//

#include "Event.hpp"

#include <iostream>

namespace sage
{
    bool Connection::IsConnected()
    {
        return id > -1 && event;
    }

    void Connection::UnSubscribe()
    {
        if (!IsConnected()) return;
        event->unSubscribe(id);
        id = -1;
        event = nullptr;
    }

    Connection::~Connection()
    {
        // UnSubscribe();
    }

    Connection::Connection(EventBase* _event, SubscriberId _id) : event(_event), id(_id)
    {
    }
} // namespace sage