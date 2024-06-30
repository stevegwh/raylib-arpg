//
// Created by steve on 08/06/2024.
//

#pragma once
#include "StatePlayerCombat.hpp"
#include "StatePlayerDefault.hpp"
#include <tuple>

namespace sage
{
	using StateComponents = std::tuple<StatePlayerDefault, StatePlayerCombat>;
}
