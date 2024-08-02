//
// Created by Steve Wheeler on 12/02/2024.
//

#include "Camera.hpp"
#include "UserInput.hpp"
#include "rcamera.h"
#include "raymath.h"

namespace sage
{
	void Camera::OnForwardKeyPressed()
	{
		forwardKeyDown = true;
	}

	void Camera::OnLeftKeyPressed()
	{
		leftKeyDown = true;
	}

	void Camera::OnRightKeyPressed()
	{
		rightKeyDown = true;
	}

	void Camera::OnBackKeyPressed()
	{
		backKeyDown = true;
	}

	void Camera::OnRotateLeftKeyPressed()
	{
		rotateLeftKeyDown = true;
	}

	void Camera::OnRotateRightKeyPressed()
	{
		rotateRightKeyDown = true;
	}

	void Camera::OnForwardKeyUp()
	{
		forwardKeyDown = false;
	}

	void Camera::OnLeftKeyUp()
	{
		leftKeyDown = false;
	}

	void Camera::OnRightKeyUp()
	{
		rightKeyDown = false;
	}

	void Camera::OnBackKeyUp()
	{
		backKeyDown = false;
	}

	void Camera::OnRotateLeftKeyUp()
	{
		rotateLeftKeyDown = false;
	}

	void Camera::OnRotateRightKeyUp()
	{
		rotateRightKeyDown = false;
	}

	void Camera::LockInput()
	{
		lockInput = true;
	}

	void Camera::UnlockInput()
	{
		lockInput = false;
	}

	void Camera::CutscenePose(const sgTransform& npcTrans)
	{
		rlCamera.position.y = rlCamera.target.y;

		// Calculate the camera's position behind the actor's shoulder
		Vector3 cameraOffset = {5.0f, 10.0f, 18.0f}; // TODO: Shouldn't be hardcoded

		Vector3 cameraPosition = Vector3Add(npcTrans.position(), cameraOffset);

		// Calculate the camera's target position slightly above the actor's position
		Vector3 cameraTarget = Vector3Add(npcTrans.position(), {0.0f, 1.0f, 0.0f});

		// Set the camera's position and target
		rlCamera.position = cameraPosition;
		rlCamera.target = cameraTarget;
	}

	void Camera::handleInput()
	{
		if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_ALT)
			|| IsKeyDown(KEY_RIGHT_ALT) || lockInput)
			return;

		if (backKeyDown)
		{
			auto right = GetCameraRight(&rlCamera);
			right = Vector3RotateByAxisAngle(right, {0, 1, 0}, DEG2RAD * 90);
			rlCamera.position = Vector3Subtract(rlCamera.position, right);
			rlCamera.target = Vector3Subtract(rlCamera.target, right);
		}

		if (forwardKeyDown)
		{
			auto right = GetCameraRight(&rlCamera);
			right = Vector3RotateByAxisAngle(right, {0, 1, 0}, DEG2RAD * 90);
			rlCamera.position = Vector3Add(right, rlCamera.position);
			rlCamera.target = Vector3Add(right, rlCamera.target);
		}

		if (leftKeyDown)
		{
			rlCamera.position = Vector3Subtract(rlCamera.position, GetCameraRight(&rlCamera));
			rlCamera.target = Vector3Subtract(rlCamera.target, GetCameraRight(&rlCamera));
		}

		if (rightKeyDown)
		{
			rlCamera.position = Vector3Add(GetCameraRight(&rlCamera), rlCamera.position);
			rlCamera.target = Vector3Add(GetCameraRight(&rlCamera), rlCamera.target);
		}

		if (rotateLeftKeyDown)
		{
			rlCamera.position = Vector3Add(GetCameraRight(&rlCamera), rlCamera.position);
		}

		if (rotateRightKeyDown)
		{
			rlCamera.position = Vector3Subtract(rlCamera.position, GetCameraRight(&rlCamera));
		}

		if (scrollEnabled)
		{
			auto mouseScroll = GetMouseWheelMoveV();
			if (mouseScroll.y > 0)
			{
				if (rlCamera.position.y > rlCamera.target.y)
				{
					Vector3 up = GetCameraUp(&rlCamera);
					up.x *= 2.0f;
					up.y *= 2.0f;
					up.z *= 2.0f;
					rlCamera.position = Vector3Subtract(rlCamera.position, up);
					rlCamera.position = Vector3Add(GetCameraForward(&rlCamera), rlCamera.position);
				}
			}
			if (mouseScroll.y < 0)
			{
				Vector3 up = GetCameraUp(&rlCamera);
				up.x *= 2.0f;
				up.y *= 2.0f;
				up.z *= 2.0f;
				rlCamera.position = Vector3Add(up, rlCamera.position);
				rlCamera.position = Vector3Subtract(rlCamera.position, GetCameraForward(&rlCamera));
			}
		}
	}

	void Camera::Update()
	{
		handleInput();
		UpdateCameraPro(&rlCamera, {0, 0, 0}, {0, 0, 0}, 0);
	}

	Camera3D* Camera::getRaylibCam()
	{
		return &rlCamera;
	}

	Camera::Camera(UserInput* userInput)
		: rlCamera({0})
	{
//		rlCamera.position = {20.0f, 40.0f, 20.0f};
//		rlCamera.target = {0.0f, 8.0f, 0.0f};
//		rlCamera.up = {0.0f, 1.0f, 0.0f};
//		rlCamera.fovy = 45.0f;

		rlCamera.position = {2.0f, 2.0f, 2.0f};
		rlCamera.target = {0.0f, 0.0f, 0.0f};
		rlCamera.up = {0.0f, 1.0f, 0.0f};
		rlCamera.fovy = 35.0f;

		//Camera camera = { { 2, 2, 2 }, { 0, 0, 0 }, { 0, 1, 0 }, 35.0, CAMERA_PERSPECTIVE };
		rlCamera.projection = CAMERA_PERSPECTIVE;

		{
			entt::sink keyWPressed{userInput->keyWPressed};
			keyWPressed.connect<&Camera::OnForwardKeyPressed>(this);
		}
		{
			entt::sink keySPressed{userInput->keySPressed};
			keySPressed.connect<&Camera::OnBackKeyPressed>(this);
		}
		{
			entt::sink keyAPressed{userInput->keyAPressed};
			keyAPressed.connect<&Camera::OnLeftKeyPressed>(this);
		}
		{
			entt::sink keyDPressed{userInput->keyDPressed};
			keyDPressed.connect<&Camera::OnRightKeyPressed>(this);
		}
		{
			entt::sink keyEPressed{userInput->keyEPressed};
			keyEPressed.connect<&Camera::OnRotateLeftKeyPressed>(this);
		}
		{
			entt::sink keyQPressed{userInput->keyQPressed};
			keyQPressed.connect<&Camera::OnRotateRightKeyPressed>(this);
		}
		{
			entt::sink keyWUp{userInput->keyWUp};
			keyWUp.connect<&Camera::OnForwardKeyUp>(this);
		}
		{
			entt::sink keySUp{userInput->keySUp};
			keySUp.connect<&Camera::OnBackKeyUp>(this);
		}
		{
			entt::sink keyAUp{userInput->keyAUp};
			keyAUp.connect<&Camera::OnLeftKeyUp>(this);
		}
		{
			entt::sink keyDUp{userInput->keyDUp};
			keyDUp.connect<&Camera::OnRightKeyUp>(this);
		}
		{
			entt::sink keyEUp{userInput->keyEUp};
			keyEUp.connect<&Camera::OnRotateLeftKeyUp>(this);
		}
		{
			entt::sink keyQUp{userInput->keyQUp};
			keyQUp.connect<&Camera::OnRotateRightKeyUp>(this);
		}
	}

	void Camera::ScrollEnable()
	{
		scrollEnabled = true;
	}

	void Camera::ScrollDisable()
	{
		scrollEnabled = false;
	}

	void Camera::SetCamera(Vector3 _pos, Vector3 _target)
	{
		rlCamera.position = _pos;
		rlCamera.target = _target;
	}
} // sage
