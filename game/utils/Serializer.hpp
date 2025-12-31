//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include "entt/entt.hpp"

namespace lq::serializer
{
    void SaveMap(entt::registry& source, const char* path);
    void LoadMap(entt::registry* destination, const char* path);
} // namespace lq::serializer
