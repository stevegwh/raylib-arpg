//
// Created by Steve Wheeler on 06/07/2024.
//

#include "slib.hpp"

namespace sage
{
Vector2 Vec3ToVec2(const Vector3& vec3)
{
    return { vec3.x, vec3.z };
}
} // sage