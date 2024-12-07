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
    void CursorClickIndicator::onCursorClick()
    {
        auto& renderable = registry->get<Renderable>(self);
        renderable.active = true;
        auto selectedActor = gameData->controllableActorSystem->GetSelectedActor();
        auto& moveable = registry->get<MoveableActor>(selectedActor);
        auto dest = moveable.GetDestination();
        auto& transform = registry->get<sgTransform>(self);
        transform.SetPosition(dest);
        entt::sink sink{moveable.onDestinationReached};
        destinationReachedCnx = sink.connect<&CursorClickIndicator::onReachLocation>(this);
    }

    void CursorClickIndicator::onSelectedActorChanged()
    {
        destinationReachedCnx.release();
        auto& renderable = registry->get<Renderable>(self);
        renderable.active = false;
    }

    void CursorClickIndicator::onReachLocation()
    {
        destinationReachedCnx.release();
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
        entt::sink sink{gameData->cursor->onFloorClick};
        sink.connect<&CursorClickIndicator::onCursorClick>(this);
        entt::sink sink2{gameData->controllableActorSystem->onSelectedActorChange};
        sink2.connect<&CursorClickIndicator::onSelectedActorChanged>(this);

        // Init indicator graphics here
        registry->emplace<sgTransform>(self, self);
        auto model = LoadModelFromMesh(GenMeshSphere(1, 32, 32));
        ModelSafe sphere(model);
        auto& renderable =
            registry->emplace<Renderable>(self, std::move(sphere), MatrixIdentity()); // requires model etc.
        renderable.hint = GREEN;
        renderable.active = false;
    }

} // namespace sage