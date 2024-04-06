//
// Created by Steve Wheeler on 06/04/2024.
//

#pragma once

#include "GameManager.hpp"

#include <memory>

#include "EventManager.hpp"
#include "AnimationController.hpp"


namespace sage
{

class PlayerAnimations : public AnimationController
{
    std::unique_ptr<EventManager> eventManager;
public:
    PlayerAnimations(EntityID _entityId, const char* _modelPath, Model* _model , const Transform* transform);
};

} // sage
