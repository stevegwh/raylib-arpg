//
// Created by steve on 05/01/2025.
//

#include "Event.hpp"

#include <iostream>

namespace sage
{

    void Connection::UnSubscribe()
    {
        if (event == nullptr)
        {
            return;
        };
        std::cout << id << std::endl;
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