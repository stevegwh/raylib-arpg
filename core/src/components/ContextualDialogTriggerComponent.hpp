//
// Created by Steve Wheeler on 03/01/2025.
//

#pragma once

namespace sage
{

    class ContextualDialogTriggerComponent
    {
        // Could target a specific player or just the leader of the party
        bool triggered = false;

      public:
        [[nodiscard]] bool CanTrigger() const
        {
            return !triggered;
        }

        void SetTriggered()
        {
            triggered = true;
        }

        float distance = 75.0f; // Distance for the dialog to trigger
    };

} // namespace sage
