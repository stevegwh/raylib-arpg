//
// Created by Steve Wheeler on 06/12/2024.
//

#include "CursorClickIndicator.hpp"

#include "components/MoveableActor.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "systems/ControllableActorSystem.hpp"

namespace sage
{

    void CursorClickIndicator::onCursorClick(entt::entity entity) const
    {
        if (entity == entt::null || !registry->any_of<Collideable>(entity)) return;
        if (const auto& col = registry->get<Collideable>(entity);
            col.collisionLayer != CollisionLayer::FLOORSIMPLE &&
            col.collisionLayer != CollisionLayer::FLOORCOMPLEX)
        {
            disableIndicator();
            return;
        }
        auto& renderable = registry->get<Renderable>(self);
        renderable.active = true;
        const auto selectedActor = gameData->controllableActorSystem->GetSelectedActor();
        const auto& moveable = registry->get<MoveableActor>(selectedActor);
        if (!moveable.IsMoving()) return;
        const auto dest = moveable.GetDestination();
        auto& transform = registry->get<sgTransform>(self);
        transform.SetPosition(dest);
    }

    void CursorClickIndicator::onSelectedActorChanged(entt::entity, entt::entity current)
    {
        if (destinationReachedCnx)
        {
            destinationReachedCnx->UnSubscribe();
        }
        auto& renderable = registry->get<Renderable>(self);
        renderable.active = false;
        auto& moveable = registry->get<MoveableActor>(current);
        destinationReachedCnx =
            moveable.onDestinationReached.Subscribe([this](entt::entity) { disableIndicator(); });
    }

    void CursorClickIndicator::disableIndicator() const
    {
        auto& renderable = registry->get<Renderable>(self);
        renderable.active = false;
    }

    void CursorClickIndicator::Update()
    {
        auto& renderable = registry->get<Renderable>(self);
        if (!renderable.active) return;
        k += 5 * GetFrameTime();
        float normalizedScale = (sin(k) + 1.0f) * 0.5f;
        float minScale = 0.25f;
        float maxScale = 1.0f;
        float scale = minScale + normalizedScale * (maxScale - minScale);

        auto& transform = registry->get<sgTransform>(self);
        transform.SetScale(scale);
    }

    CursorClickIndicator::CursorClickIndicator(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData), self(registry->create())
    {
        _gameData->cursor->onAnyLeftClick.Subscribe([this](const entt::entity entity) { onCursorClick(entity); });
        _gameData->controllableActorSystem->onSelectedActorChange.Subscribe(
            [this](entt::entity prev, entt::entity current) { onSelectedActorChanged(prev, current); });

        // Init indicator graphics here
        _registry->emplace<sgTransform>(self, self);
        auto model = LoadModelFromMesh(GenMeshSphere(1, 32, 32));
        ModelSafe sphere(model);
        auto& renderable =
            _registry->emplace<Renderable>(self, std::move(sphere), MatrixIdentity()); // requires model etc.
        renderable.hint = GREEN;
        renderable.active = false;
    }

} // namespace sage