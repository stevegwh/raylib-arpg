//
// Created by Steve Wheeler on 31/07/2024.
//

#pragma once

#include "StateGameDefault.hpp"
#include "StateGameWaveattack.hpp"
#include <tuple>

namespace sage
{
	using StateComponents = std::tuple<StateGameDefault, StateGameWaveattack>;
}