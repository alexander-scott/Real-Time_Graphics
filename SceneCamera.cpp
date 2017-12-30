//***************************************************************************************
// Camera.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "SceneCamera.h"

using namespace DirectX;

SceneCamera::SceneCamera(float nearDepth, float farDepth, float windowWidth, float windowHeight, bool canFly)
{
	mFovY = 0.25f * kPI;
	mAspect = windowWidth / windowHeight;
	mNearZ = nearDepth;
	mFarZ = farDepth;

	mCanFly = canFly;

	UpdateCameraViewMatrix();
	CreateProjectionMatrix();
}

SceneCamera::~SceneCamera()
{
}

BoundingFrustum SceneCamera::GetBoundingFrustum()
{
	XMVECTOR detView = XMMatrixDeterminant(GetViewMatrix());
	XMMATRIX invView = XMMatrixInverse(&detView, GetViewMatrix());

	// Get the walking cameras frustum
	BoundingFrustum worldSpaceFrustum = BoundingFrustum(mProjectionMatrix);

	// Move the frustum to the view matrix position
	BoundingFrustum localSpaceFrustum;
	worldSpaceFrustum.Transform(localSpaceFrustum, invView);

	return localSpaceFrustum;
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

	mProjectionMatrix = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
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

		if (GetAsyncKeyState(VK_UP))
		{
			mForwardVelocity += kCameraMoveSpeed;
		}
		else if (GetAsyncKeyState(VK_DOWN))
		{
			mForwardVelocity -= kCameraMoveSpeed;
		}

		if (GetAsyncKeyState(VK_LEFT))
		{
			mStrafeVelocity -= kCameraMoveSpeed;
		}
		else if (GetAsyncKeyState(VK_RIGHT))
		{
			mStrafeVelocity += kCameraMoveSpeed;
		}
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void SceneCamera::UpdateCameraViewMatrix()
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
	XMVECTOR camRight = XMVector3TransformCoord(XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), rotateYTempMatrix);
	XMVECTOR camForward = XMVector3TransformCoord(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), rotateYTempMatrix);
	XMVECTOR camUp = XMVector3TransformCoord(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), rotateYTempMatrix);
	
	// Load the stored camera pos in as a vector
	XMVECTOR camPos = XMLoadFloat3(&mCameraPos);

	// Move the position forward/back or left/right depending on user input
	camPos += mStrafeVelocity * camRight;
	if (mCanFly)
		camPos += mForwardVelocity * camTarget;
	else
		camPos += mForwardVelocity * camForward;

	mStrafeVelocity = 0.0f;
	mForwardVelocity = 0.0f;

	// Move the target by the change in position
	camTarget = camPos + camTarget;

	// Store the new camera position
	XMStoreFloat3(&mCameraPos, camPos);

	// Generate and store the new view matrix based on the camera position and target and up vectors
	mViewMatrix = XMMatrixLookAtLH(camPos, camTarget, camUp);
}