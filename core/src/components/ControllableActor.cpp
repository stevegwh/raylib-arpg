#include "ControllableActor.hpp"

#include "EntityReflectionSignalRouter.hpp"
#include "TextureTerrainOverlay.hpp" // used for construction

namespace sage
{
    void ControllableActor::AddHook(const int id)
    {
        hooks.push_back(id);
    }

    void ControllableActor::ReleaseAllHooks(EntityReflectionSignalRouter* router)
    {
        for (const auto hook : hooks)
        {
            router->RemoveHook(hook);
        }
        hooks.clear();
    }

    ControllableActor::ControllableActor(entt::entity _self) : self(_self)
    {
    }
} // namespace sage