//
// Created by Steve Wheeler on 29/12/2024.
//

#include "DoorBehaviorComponent.hpp"

#include "components/sgTransform.hpp"
#include "Cursor.hpp"

namespace sage
{

    void DoorBehaviorComponent::ExecuteBehavior(entt::entity clicked)
    {
        if (clicked != self || locked) return;

        static const float closedRotation = 0.0f;

        const auto& [rotx, roty, rotz] = transform->GetLocalRot();

        if (!open) // Opening the door
        {
            // If facing positive Z, rotate one way; if facing negative Z, rotate the other
            float targetRotation = (transform->forward().z > 0) ? openYRotation : -openYRotation;
            transform->SetLocalRot(Vector3{rotx, targetRotation, rotz});
            open = true;
        }
        else // Closing the door
        {
            // Always return to the original rotation when closing
            transform->SetLocalRot(Vector3{rotx, closedRotation, rotz});
            open = false;
        }
    }

    DoorBehaviorComponent::~DoorBehaviorComponent()
    {
        connection.release();
    }

    DoorBehaviorComponent::DoorBehaviorComponent(entt::entity _self, sgTransform* _transform)
        : self(_self), transform(_transform)
    {
    }
} // namespace sage