#include "ControllableActor.hpp"

#include "Cursor.hpp"

namespace sage
{
    ControllableActor::ControllableActor(entt::entity _self, Cursor* _cursor) : self(_self)
    {
    }
} // namespace sage