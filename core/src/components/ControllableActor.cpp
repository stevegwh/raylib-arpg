#include "ControllableActor.hpp"

#include "Cursor.hpp"

namespace sage
{
    void ControllableActor::EnemyClicked(entt::entity enemy)
    {
        assert(enemy != entt::null);
        onEnemyClicked.publish(self, enemy);
    }

    ControllableActor::ControllableActor(entt::entity _self, Cursor* _cursor)
        : self(_self)
    {
        entt::sink sink{_cursor->onEnemyClick};
        sink.connect<&ControllableActor::EnemyClicked>(this);
        checkTargetPosTimer.SetMaxTime(0.5f);
    }
} // namespace sage