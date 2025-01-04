//
// Created by steve on 04/01/2025.
//

#pragma once

#include <string>

namespace sage
{

    class SpatialAudioComponent
    {
      public:
        std::string audioKey;
        float maxDistance = 50;
    };

} // namespace sage
