//
// Created by steve on 05/01/2025.
//

#include "Event.hpp"

namespace sage
{

    void Connection::UnSubscribe()
    {
        if (event != nullptr && id >= 0) return;
        event->unSubscribe(id);
        id = -1;
        event = nullptr;
    }

    Connection::Connection(EventBase* _event, SubscriberId _id) : event(_event), id(_id)
    {
    }
} // namespace sage