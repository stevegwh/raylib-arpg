//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "cereal/cereal.hpp"


struct Settings
{
    int SCREEN_WIDTH = 1280;
    int SCREEN_HEIGHT = 720;

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(SCREEN_WIDTH),
                CEREAL_NVP(SCREEN_HEIGHT));
    }
};
