//
// Created by Steve Wheeler on 03/05/2024.
//

#include "sgTransform.hpp"
#include "raymath.h"

namespace sage
{
	Matrix sgTransform::GetMatrixNoRot() const
	{
		Matrix trans = MatrixTranslate(m_position.x, m_position.y, m_position.z);
		Matrix _scale = MatrixScale(m_scale, m_scale, m_scale);
		//Matrix rot = MatrixRotateXYZ({DEG2RAD*transform->rotation.x, DEG2RAD*transform->rotation.y, DEG2RAD*transform->rotation.z});
		return MatrixMultiply(trans, _scale);
	}

    Matrix sgTransform::GetMatrix() const
    {
        Matrix trans = MatrixTranslate(m_position.x, m_position.y, m_position.z);
        Matrix _scale = MatrixScale(m_scale, m_scale, m_scale);
        Matrix rot = MatrixRotateXYZ({DEG2RAD * m_rotation.x, DEG2RAD * m_rotation.y, DEG2RAD * m_rotation.z});
        return MatrixMultiply(MatrixMultiply(trans, rot), _scale);
    }

	Vector3 sgTransform::forward() const
	{
		Matrix matrix = GetMatrix();
		Vector3 forward = {matrix.m8, matrix.m9, matrix.m10};
		return Vector3Normalize(forward);
	}

	const Vector3& sgTransform::position() const
	{
		return m_position;
	}

	const Vector3& sgTransform::rotation() const
	{
		return m_rotation;
	}

	float sgTransform::scale() const
	{
		return m_scale;
	}

	void sgTransform::SetPosition(const Vector3& position, const entt::entity& entity)
	{
		m_position = position;
		onPositionUpdate.publish(entity);
	}

	void sgTransform::SetRotation(const Vector3& rotation, const entt::entity& entity)
	{
		m_rotation = rotation;;
	}

	void sgTransform::SetScale(float scale, const entt::entity& entity)
	{
		m_scale = scale;
	}
}
