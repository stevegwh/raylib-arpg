//
// Created by Steve Wheeler on 03/05/2024.
//

#include "Transform.hpp"
#include "raymath.h"

namespace sage
{
	Matrix Transform::GetMatrixNoRot() const
	{
		Matrix trans = MatrixTranslate(position.x, position.y, position.z);
		Matrix _scale = MatrixScale(scale, scale, scale);
		//Matrix rot = MatrixRotateXYZ({DEG2RAD*transform->rotation.x, DEG2RAD*transform->rotation.y, DEG2RAD*transform->rotation.z});
		return MatrixMultiply(trans, _scale);
	}

    Matrix Transform::GetMatrix() const
    {
        Matrix trans = MatrixTranslate(position.x, position.y, position.z);
        Matrix _scale = MatrixScale(scale, scale, scale);
        Matrix rot = MatrixRotateXYZ({DEG2RAD * rotation.x, DEG2RAD * rotation.y, DEG2RAD * rotation.z});
        return MatrixMultiply(MatrixMultiply(trans, rot), _scale);
    }

	Vector3 Transform::forward() const
	{
		Matrix matrix = GetMatrix();
		Vector3 forward = {matrix.m8, matrix.m9, matrix.m10};
		return Vector3Normalize(forward);
	}
}
