//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include "raylib.h"

#include "Entity.hpp"



namespace sage
{

struct GameObjectFactory
{
    static void createTower(Vector3 position, const char* name) ;
    static EntityID createPlayer(Vector3 position, const char* name) ;

};

} // sage
