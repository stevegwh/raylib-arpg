#include "systems/CursorClickIndicator.hpp"

#include "Systems.hpp"

#include "engine/components/MoveableActor.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/Cursor.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/systems/NavigationGridSystem.hpp"
#include "engine/systems/TransformSystem.hpp"

#include "raylib.h"
#include "raymath.h"

namespace lq
{
    void CursorClickIndicator::onCursorClick(entt::entity entity, sage::CollisionLayer layer) const
    {
        const auto& navHitInfo = sys->engine.cursor->getNavigationHitInfo();
        if (selectedActor == entt::null || !registry->valid(selectedActor) ||
            !registry->all_of<sage::MoveableActor>(selectedActor) || !navHitInfo.rlCollision.hit ||
            navHitInfo.collidedEntityId == entt::null ||
            !sys->engine.navigationGridSystem->IsValidMove(navHitInfo.rlCollision.point, selectedActor))
        {
            disableIndicator();
            return;
        }

        if (!sage::IsNavigationLayer(navHitInfo.collisionLayer))
        {
            disableIndicator();
            return;
        }
        const auto& moveable = registry->get<sage::MoveableActor>(selectedActor);
        if (!moveable.IsMoving())
        {
            disableIndicator();
            return;
        }

        auto& renderable = registry->get<sage::Renderable>(self);
        renderable.active = true;
        sys->engine.transformSystem->SetWorldPos(self, moveable.GetDestination());
    }

    void CursorClickIndicator::OnSelectedActorChanged(entt::entity, entt::entity current)
    {
        selectedActor = current;
        if (destinationReachedSub.IsActive())
        {
            destinationReachedSub.UnSubscribe();
        }

        auto& renderable = registry->get<sage::Renderable>(self);
        renderable.active = false;
        if (current == entt::null || !registry->valid(current) || !registry->all_of<sage::MoveableActor>(current))
        {
            return;
        }

        auto& moveable = registry->get<sage::MoveableActor>(current);
        destinationReachedSub = moveable.onDestinationReached.Subscribe([this](entt::entity) { disableIndicator(); });
    }

    void CursorClickIndicator::disableIndicator() const
    {
        auto& renderable = registry->get<sage::Renderable>(self);
        renderable.active = false;
    }

    void CursorClickIndicator::Update()
    {
        auto& renderable = registry->get<sage::Renderable>(self);
        if (!renderable.active) return;

        k += 5.0f * GetFrameTime();
        constexpr float minScale = 0.25f;
        constexpr float maxScale = 1.0f;
        const float normalizedScale = (sin(k) + 1.0f) * 0.5f;
        const float scale = minScale + normalizedScale * (maxScale - minScale);

        sys->engine.transformSystem->SetWorldScale(self, scale);
    }

    CursorClickIndicator::CursorClickIndicator(entt::registry* _registry, Systems* _sys)
        : registry(_registry), sys(_sys), self(registry->create())
    {
        cursorClickSub = _sys->engine.cursor->onLeftClick.Subscribe(
            [this](entt::entity entity, sage::CollisionLayer layer) { onCursorClick(entity, layer); });

        _registry->emplace<sage::sgTransform>(self);
        auto sphere = sage::ResourceManager::GetInstance().CreateModelMutable("primitive_sphere");
        auto& renderable = _registry->emplace<sage::Renderable>(self, std::move(sphere), MatrixIdentity());
        renderable.hint = GREEN;
        renderable.active = false;
    }
} // namespace lq
