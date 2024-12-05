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
            Lit = 1 << 1,
            Emissive = 1 << 2
        };

        // uint32_t flags{};
        Shader shader{};
        int litLoc{};
        int skinnedLoc{};
        int emissiveLoc{}; // The boolean, not the texture

        std::vector<uint32_t> materialMap;

        void SetShaderLocs() const
        {
            for (unsigned int i = 0; i < materialMap.size(); ++i)
            {
                int valueT = 1;
                int valueF = 0;
                if (HasFlag(i, Skinned))
                {
                    SetShaderValue(shader, skinnedLoc, &valueT, RL_SHADER_UNIFORM_INT);
                }
                else
                {
                    SetShaderValue(shader, skinnedLoc, &valueF, RL_SHADER_UNIFORM_INT);
                }
                if (HasFlag(i, Lit))
                {
                    SetShaderValue(shader, litLoc, &valueT, RL_SHADER_UNIFORM_INT);
                }
                else
                {
                    SetShaderValue(shader, litLoc, &valueF, RL_SHADER_UNIFORM_INT);
                }

                if (HasFlag(i, Emissive))
                {
                    SetShaderValue(shader, emissiveLoc, &valueT, RL_SHADER_UNIFORM_INT);
                }
                else
                {
                    SetShaderValue(shader, emissiveLoc, &valueF, RL_SHADER_UNIFORM_INT);
                }
            }
        }

        [[nodiscard]] bool HasFlag(unsigned int idx, Flags flag) const
        {
            return materialMap.at(idx) & flag;
        }

        void SetFlag(unsigned int idx, Flags flag)
        {
            materialMap.at(idx) |= flag;
        }

        void ClearFlag(unsigned int idx, Flags flag)
        {
            materialMap.at(idx) &= ~flag;
        }

        void SetFlagAll(Flags flag)
        {
            for (unsigned int& i : materialMap)
            {
                i |= flag;
            }
        }

        void ClearFlagAll(Flags flag)
        {
            for (unsigned int& i : materialMap)
            {
                i &= ~flag;
            }
        }

        explicit UberShaderComponent(unsigned int materialCount)
        {
            materialMap.resize(materialCount);
        }
    };

}; // namespace sage