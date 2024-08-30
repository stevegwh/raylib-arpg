#include "ControllableActor.hpp"

#include "Cursor.hpp"

namespace sage
{
    ControllableActor::ControllableActor(entt::entity _self, Cursor* _cursor) : self(_self)
    {
        checkTargetPosTimer.SetMaxTime(0.5f);
    }
} // namespace sage