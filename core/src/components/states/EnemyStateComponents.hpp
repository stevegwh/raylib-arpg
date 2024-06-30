//
// Created by steve on 08/06/2024.
//
#pragma once
#include "StateEnemyCombat.hpp"
#include "StateEnemyDefault.hpp"
#include "tuple"

namespace sage
{
using StateComponents = std::tuple<StateEnemyDefault, StateEnemyCombat>;
}

