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
            EmissiveTexture = 1 << 2,
            EmissiveCol = 1 << 3
        };

        // uint32_t flags{};
        Shader shader{};
        int litLoc{};
        int skinnedLoc{};
        int emissiveTexLoc{}; // The boolean, not the texture
        int emissiveColLoc{};

        std::vector<uint32_t> materialMap;

        // TODO: Change this to "SetShaderBools"
        void SetShaderLocs(unsigned int materialIdx) const
        {
            int valueT = 1;
            int valueF = 0;
            if (HasFlag(materialIdx, Skinned))
            {
                SetShaderValue(shader, skinnedLoc, &valueT, RL_SHADER_UNIFORM_INT);
            }
            else
            {
                SetShaderValue(shader, skinnedLoc, &valueF, RL_SHADER_UNIFORM_INT);
            }
            if (HasFlag(materialIdx, Lit))
            {
                SetShaderValue(shader, litLoc, &valueT, RL_SHADER_UNIFORM_INT);
            }
            else
            {
                SetShaderValue(shader, litLoc, &valueF, RL_SHADER_UNIFORM_INT);
            }

            if (HasFlag(materialIdx, EmissiveTexture))
            {
                SetShaderValue(shader, emissiveTexLoc, &valueT, RL_SHADER_UNIFORM_INT);
            }
            else
            {
                SetShaderValue(shader, emissiveTexLoc, &valueF, RL_SHADER_UNIFORM_INT);
            }
            if (HasFlag(materialIdx, EmissiveCol))
            {
                SetShaderValue(shader, emissiveColLoc, &valueT, RL_SHADER_UNIFORM_INT);
            }
            else
            {
                SetShaderValue(shader, emissiveColLoc, &valueF, RL_SHADER_UNIFORM_INT);
            }
        }

        void SetShaderLocs() const
        {
            for (unsigned int i = 0; i < materialMap.size(); ++i)
            {
                SetShaderLocs(i);
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