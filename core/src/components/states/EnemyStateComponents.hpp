//
// Created by steve on 08/06/2024.
//

#include "StateEnemyCombat.hpp"
#include "StateEnemyDefault.hpp"
#include "tuple"

#pragma once
namespace sage
{
using StateComponents = std::tuple<StateEnemyDefault, StateEnemyCombat>;
}

