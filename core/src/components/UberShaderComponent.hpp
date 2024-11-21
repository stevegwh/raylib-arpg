//
// Created by steve on 21/11/2024.
//

#pragma once

#include "raylib.h"

namespace sage
{
    struct UberShaderComponent
    {
        enum Flags
        {
            Skinned = 1 << 0,
            Lit = 1 << 1
        };

        uint32_t flags;
        Shader shader{};
        int litLoc{};
        int skinnedLoc{};

        void SetShaderLocs() const
        {
            int valueT = 1;
            int valueF = 0;
            if (HasFlag(Skinned))
            {
                SetShaderValue(shader, skinnedLoc, &valueT, RL_SHADER_UNIFORM_INT);
            }
            else
            {
                SetShaderValue(shader, skinnedLoc, &valueF, RL_SHADER_UNIFORM_INT);
            }
            if (HasFlag(Lit))
            {
                SetShaderValue(shader, litLoc, &valueT, RL_SHADER_UNIFORM_INT);
            }
            else
            {
                SetShaderValue(shader, litLoc, &valueF, RL_SHADER_UNIFORM_INT);
            }
        }

        [[nodiscard]] bool HasFlag(Flags flag) const
        {
            return flags & flag;
        }

        void SetFlag(Flags flag)
        {
            flags |= flag;
        }

        void ClearFlag(Flags flag)
        {
            flags &= ~flag;
        }

        explicit UberShaderComponent(uint32_t initialFlags = 0) : flags(initialFlags)
        {
        }
    };

}; // namespace sage