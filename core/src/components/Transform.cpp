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
    return MatrixMultiply(_scale, trans);
}

Matrix Transform::GetMatrix() const
{
    Matrix trans = MatrixTranslate(position.x, position.y, position.z);
    Matrix _scale = MatrixScale(scale, scale, scale);
    Matrix rot = MatrixRotateXYZ({DEG2RAD*rotation.x, DEG2RAD*rotation.y, DEG2RAD*rotation.z});
    return MatrixMultiply(_scale, MatrixMultiply(rot, trans));
}
}
