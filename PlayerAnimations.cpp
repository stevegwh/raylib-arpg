//
// Created by Steve Wheeler on 06/04/2024.
//

#include "PlayerAnimations.hpp"

namespace sage
{
PlayerAnimations::PlayerAnimations(EntityID _entityId, const char *_modelPath, Model *_model, const Transform* transform)
    : eventManager(std::make_unique<EventManager>()), AnimationController(_entityId, _modelPath, _model, transform)
{
    eventManager->Subscribe( [p = this] { p->changeAnimation(3); }, *transform->OnStartMovement);
    eventManager->Subscribe( [p = this] { p->changeAnimation(0); }, *transform->OnFinishMovement);
}
} // sage