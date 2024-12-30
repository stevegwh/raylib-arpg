//
// Created by Steve Wheeler on 29/12/2024.
//

#include "DoorBehaviorComponent.hpp"

#include "components/sgTransform.hpp"
#include "Cursor.hpp"

namespace sage
{

    void DoorBehaviorComponent::UnlockDoor()
    {
        locked = false;
    }

    void DoorBehaviorComponent::UnlockAndOpenDoor()
    {
        UnlockDoor();
        ExecuteBehavior(self);
    }

    // TODO: This is essentially hard coded to open the sole door in the game right now.
    void DoorBehaviorComponent::ExecuteBehavior(entt::entity clicked)
    {
        // TODO: Likely, the issue with rotation stems from the fact that its position and rotation are currently
        // set by the transform matrix, it's world position/rotation is currently set to zero.
        if (clicked != self || locked) return;

        static const float closedRotation = 0.0f;

        const auto& [rotx, roty, rotz] = transform->GetLocalRot();

        if (!open)
        {
            float targetRotation = (transform->forward().z > 0) ? openYRotation : -openYRotation;
            transform->SetLocalRot(Vector3{rotx, targetRotation, rotz});
            open = true;
        }
        else
        {
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