#include "Transform.hpp"
#include "Transform.hpp"
#include "Transform.hpp"
#include "Transform.hpp"
#include "Transform.hpp"
//
// Created by Steve Wheeler on 03/05/2024.
//

#include "Transform.hpp"
#include "raymath.h"

namespace sage
{
	Matrix Transform::GetMatrixNoRot() const
	{
		Matrix trans = MatrixTranslate(m_position.x, m_position.y, m_position.z);
		Matrix _scale = MatrixScale(m_scale, m_scale, m_scale);
		//Matrix rot = MatrixRotateXYZ({DEG2RAD*transform->rotation.x, DEG2RAD*transform->rotation.y, DEG2RAD*transform->rotation.z});
		return MatrixMultiply(trans, _scale);
	}

    Matrix Transform::GetMatrix() const
    {
        Matrix trans = MatrixTranslate(m_position.x, m_position.y, m_position.z);
        Matrix _scale = MatrixScale(m_scale, m_scale, m_scale);
        Matrix rot = MatrixRotateXYZ({DEG2RAD * m_rotation.x, DEG2RAD * m_rotation.y, DEG2RAD * m_rotation.z});
        return MatrixMultiply(MatrixMultiply(trans, rot), _scale);
    }

	Vector3 Transform::forward() const
	{
		Matrix matrix = GetMatrix();
		Vector3 forward = {matrix.m8, matrix.m9, matrix.m10};
		return Vector3Normalize(forward);
	}

	const Vector3& Transform::position() const
	{
		return m_position;
	}

	const Vector3& Transform::rotation() const
	{
		return m_rotation;
	}

	float Transform::scale() const
	{
		return m_scale;
	}

	void Transform::SetPosition(const Vector3& position, const entt::entity& entity)
	{
		m_position = position;
		onPositionUpdate.publish(entity);
	}

	void Transform::SetRotation(const Vector3& rotation, const entt::entity& entity)
	{
		m_rotation = rotation;;
	}

	void Transform::SetScale(float scale, const entt::entity& entity)
	{
		m_scale = scale;
	}
}
