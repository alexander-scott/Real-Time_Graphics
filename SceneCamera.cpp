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

std::vector<XMFLOAT4> SceneCamera::GetFrustumPlanes()
{
	// x, y, z, and w represent A, B, C and D in the plane equation
	// where ABC are the xyz of the planes normal, and D is the plane constant
	std::vector<XMFLOAT4> tempFrustumPlane(6);

	// Left Frustum Plane
	// Add first column of the matrix to the fourth column
	tempFrustumPlane[0].x = mViewMatrix._14 + mViewMatrix._11;
	tempFrustumPlane[0].y = mViewMatrix._24 + mViewMatrix._21;
	tempFrustumPlane[0].z = mViewMatrix._34 + mViewMatrix._31;
	tempFrustumPlane[0].w = mViewMatrix._44 + mViewMatrix._41;

	// Right Frustum Plane
	// Subtract first column of matrix from the fourth column
	tempFrustumPlane[1].x = mViewMatrix._14 - mViewMatrix._11;
	tempFrustumPlane[1].y = mViewMatrix._24 - mViewMatrix._21;
	tempFrustumPlane[1].z = mViewMatrix._34 - mViewMatrix._31;
	tempFrustumPlane[1].w = mViewMatrix._44 - mViewMatrix._41;

	// Top Frustum Plane
	// Subtract second column of matrix from the fourth column
	tempFrustumPlane[2].x = mViewMatrix._14 - mViewMatrix._12;
	tempFrustumPlane[2].y = mViewMatrix._24 - mViewMatrix._22;
	tempFrustumPlane[2].z = mViewMatrix._34 - mViewMatrix._32;
	tempFrustumPlane[2].w = mViewMatrix._44 - mViewMatrix._42;

	// Bottom Frustum Plane
	// Add second column of the matrix to the fourth column
	tempFrustumPlane[3].x = mViewMatrix._14 + mViewMatrix._12;
	tempFrustumPlane[3].y = mViewMatrix._24 + mViewMatrix._22;
	tempFrustumPlane[3].z = mViewMatrix._34 + mViewMatrix._32;
	tempFrustumPlane[3].w = mViewMatrix._44 + mViewMatrix._42;

	// Near Frustum Plane
	// We could add the third column to the fourth column to get the near plane,
	// but we don't have to do this because the third column IS the near plane
	tempFrustumPlane[4].x = mViewMatrix._13;
	tempFrustumPlane[4].y = mViewMatrix._23;
	tempFrustumPlane[4].z = mViewMatrix._33;
	tempFrustumPlane[4].w = mViewMatrix._43;

	// Far Frustum Plane
	// Subtract third column of matrix from the fourth column
	tempFrustumPlane[5].x = mViewMatrix._14 - mViewMatrix._13;
	tempFrustumPlane[5].y = mViewMatrix._24 - mViewMatrix._23;
	tempFrustumPlane[5].z = mViewMatrix._34 - mViewMatrix._33;
	tempFrustumPlane[5].w = mViewMatrix._44 - mViewMatrix._43;

	// Normalize plane normals (A, B and C (xyz))
	// Also take note that planes face inward
	for (int i = 0; i < 6; ++i)
	{
		float length = sqrt((tempFrustumPlane[i].x * tempFrustumPlane[i].x) + (tempFrustumPlane[i].y * tempFrustumPlane[i].y) + (tempFrustumPlane[i].z * tempFrustumPlane[i].z));
		tempFrustumPlane[i].x /= length;
		tempFrustumPlane[i].y /= length;
		tempFrustumPlane[i].z /= length;
		tempFrustumPlane[i].w /= length;
	}

	return tempFrustumPlane;
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