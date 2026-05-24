//
// Created by Steve Wheeler on 24/05/2026.
//

#pragma once
namespace sage
{
    template <typename Derived>
    struct Component
    {
        bool dirty = false;
        void markDirty()
        {
            dirty = true;
        }
    };
} // namespace sage
