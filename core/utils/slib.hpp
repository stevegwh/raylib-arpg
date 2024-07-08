//
// Created by Steve Wheeler on 06/07/2024.
//

#pragma once

#include "raylib.h"

namespace sage
{
	Vector2 Vec3ToVec2(const Vector3& vec3);
	BoundingBox CalculateModelBoundingBox(Model& model);
} // sage
