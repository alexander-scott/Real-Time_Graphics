//***************************************************************************************
// Camera.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "SceneCamera.h"

using namespace DirectX;

SceneCamera::SceneCamera(float nearDepth, float farDepth, float windowWidth, float windowHeight)
{
	mFovY = 0.25f * kPI;
	mAspect = windowWidth / windowHeight;
	mNearZ = nearDepth;
	mFarZ = farDepth;

	UpdateCameraView();
	CreateProjectionMatrix();
}

SceneCamera::~SceneCamera()
{
}

void SceneCamera::SetPosition(float x, float y, float z)
{
	mCameraPos = XMFLOAT3(x, y, z);
}

void SceneCamera::SetPosition(const XMFLOAT3& v)
{
	mCameraPos = v;
}

// This function needs to be called if the window size changes
void SceneCamera::CreateProjectionMatrix()
{
	mNearWindowHeight = 2.0f * mNearZ * tanf(0.5f*mFovY);
	mFarWindowHeight = 2.0f * mFarZ * tanf(0.5f*mFovY);

	XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	XMStoreFloat4x4(&mProjectionMatrix, P);
}

void SceneCamera::OnMouseMove(int x, int y)
{
	// If right mouse button pressed
	if (GetAsyncKeyState(MK_RBUTTON))
	{
		// Pitch
		mCameraPitch = mCameraPitch + XMConvertToRadians(kCameraLookSpeed*static_cast<float>(y - mLastMousePos.y));
		// Yaw
		mCameraYaw = mCameraYaw + XMConvertToRadians(kCameraLookSpeed*static_cast<float>(x - mLastMousePos.x));

		// Don't let the user look too far up or down
		mCameraPitch = min(kCameraMaxPitch, mCameraPitch);
		mCameraPitch = max(-kCameraMaxPitch, mCameraPitch);

		// If the user looks too far left or right reset it so can continue moving in that direction allowing for 360 degree movement
		/*if (mCameraYaw > kPI)
			mCameraYaw -= kPI * 2;
		else if (mCameraYaw <= -kPI)
			mCameraYaw += kPI * 2;*/

		if (GetAsyncKeyState(VK_UP))
		{
			moveBackForward += kCameraMoveSpeed;
		}
		else if (GetAsyncKeyState(VK_DOWN))
		{
			moveBackForward -= kCameraMoveSpeed;
		}

		if (GetAsyncKeyState(VK_LEFT))
		{
			moveLeftRight -= kCameraMoveSpeed;
		}
		else if (GetAsyncKeyState(VK_RIGHT))
		{
			moveLeftRight += kCameraMoveSpeed;
		}
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void SceneCamera::UpdateCameraView()
{
	// Generate a rotation matrix based on the current pitch and yaw of the camera
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(mCameraPitch, mCameraYaw, 0);

	// Calculate the target by multiplying the default forward vector by the rotation matrix
	XMVECTOR camTarget = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), camRotationMatrix);
	camTarget = XMVector3Normalize(camTarget);

	// Generate a rotation matrix just based on yaw
	XMMATRIX rotateYTempMatrix;
	rotateYTempMatrix = XMMatrixRotationY(mCameraYaw);

	// Calculate the right, forward and up vectors by multiplying their defaults vectors by the rotation matrix
	camRight = XMVector3TransformCoord(XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), rotateYTempMatrix);
	camForward = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), rotateYTempMatrix);
	camUp = XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), rotateYTempMatrix);
	
	// Load the stored camera pos in as a vector
	XMVECTOR camPos = XMLoadFloat3(&mCameraPos);

	// Move the position forward/back or left/right depending on user input
	camPos += moveLeftRight * camRight;
	camPos += moveBackForward * camTarget; // CHANGE THIS TO camForward TO ENABLE WALKING

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;

	// Move the target by the change in position
	camTarget = camPos + camTarget;

	// Store the new camera position
	XMStoreFloat3(&mCameraPos, camPos);

	// Generate and store the new view matrix based on the camera position and target and up vectors
	XMStoreFloat4x4(&mViewMatrix, XMMatrixLookAtLH(camPos, camTarget, camUp));
}