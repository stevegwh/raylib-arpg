//
// Created by steve on 08/06/2024.
//

#include "components/states/StateEnemyCombat.hpp"
#include "components/states/StateEnemyDefault.hpp"
#include "tuple"

#pragma once
namespace sage
{
using StateComponents = std::tuple<StateEnemyDefault, StateEnemyCombat>;
}

