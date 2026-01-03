//
// Created by Steve Wheeler on 06/12/2024.
//

#include "CursorClickIndicator.hpp"

#include "engine/components/MoveableActor.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/Cursor.hpp"
#include "Systems.hpp"
#include "systems/ControllableActorSystem.hpp"

namespace lq
{

    void CursorClickIndicator::onCursorClick(entt::entity entity) const
    {
        if (entity == entt::null || !registry->any_of<sage::Collideable>(entity) || !sys->cursor->IsValidMove())
        {
            disableIndicator();
            return;
        }

        const auto& col = registry->get<sage::Collideable>(entity);
        if (col.collisionLayer != sage::CollisionLayer::GEOMETRY_SIMPLE &&
            col.collisionLayer != sage::CollisionLayer::GEOMETRY_COMPLEX)
        {
            disableIndicator();
            return;
        }

        const auto selectedActor = sys->cursor->GetSelectedActor();
        const auto& moveable = registry->get<sage::MoveableActor>(selectedActor);
        if (!moveable.IsMoving())
        {
            disableIndicator();
            return;
        }
        auto& renderable = registry->get<sage::Renderable>(self);
        renderable.active = true;
        const auto dest = moveable.GetDestination();
        auto& transform = registry->get<sage::sgTransform>(self);
        transform.SetPosition(dest);
    }

    void CursorClickIndicator::onSelectedActorChanged(entt::entity, entt::entity current)
    {
        if (destinationReachedSub.IsActive())
        {
            destinationReachedSub.UnSubscribe();
        }
        auto& renderable = registry->get<sage::Renderable>(self);
        renderable.active = false;
        auto& moveable = registry->get<sage::MoveableActor>(current);
        destinationReachedSub =
            moveable.onDestinationReached.Subscribe([this](entt::entity) { disableIndicator(); });
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
        k += 5 * GetFrameTime();
        float normalizedScale = (sin(k) + 1.0f) * 0.5f;
        float minScale = 0.25f;
        float maxScale = 1.0f;
        float scale = minScale + normalizedScale * (maxScale - minScale);

        auto& transform = registry->get<sage::sgTransform>(self);
        transform.SetScale(scale);
    }

    CursorClickIndicator::CursorClickIndicator(entt::registry* _registry, Systems* _sys)
        : registry(_registry), sys(_sys), self(registry->create())
    {
        _sys->cursor->onAnyLeftClick.Subscribe([this](const entt::entity entity) { onCursorClick(entity); });
        _sys->cursor->onSelectedActorChange.Subscribe(
            [this](entt::entity prev, entt::entity current) { onSelectedActorChanged(prev, current); });

        // Init indicator graphics here
        _registry->emplace<sage::sgTransform>(self, self);
        auto model = LoadModelFromMesh(GenMeshSphere(1, 32, 32));
        sage::ModelSafe sphere(model);
        auto& renderable =
            _registry->emplace<sage::Renderable>(self, std::move(sphere), MatrixIdentity()); // requires model etc.
        renderable.hint = GREEN;
        renderable.active = false;
    }

} // namespace lq