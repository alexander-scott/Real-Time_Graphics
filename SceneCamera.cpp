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

	mProjectionMatrix = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
}

BoundingFrustum SceneCamera::GetBoundingFrustum()
{
	return BoundingFrustum(mProjectionMatrix);
}

std::vector<XMFLOAT4> SceneCamera::GetFrustumPlanes()
{
	XMFLOAT4X4 viewMat = XMMATRIXToXMFLOAT4X4(mViewMatrix);

	// x, y, z, and w represent A, B, C and D in the plane equation
	// where ABC are the xyz of the planes normal, and D is the plane constant
	std::vector<XMFLOAT4> tempFrustumPlane(6);

	// Left Frustum Plane
	// Add first column of the matrix to the fourth column
	tempFrustumPlane[0].x = viewMat._14 + viewMat._11;
	tempFrustumPlane[0].y = viewMat._24 + viewMat._21;
	tempFrustumPlane[0].z = viewMat._34 + viewMat._31;
	tempFrustumPlane[0].w = viewMat._44 + viewMat._41;

	// Right Frustum Plane
	// Subtract first column of matrix from the fourth column
	tempFrustumPlane[1].x = viewMat._14 - viewMat._11;
	tempFrustumPlane[1].y = viewMat._24 - viewMat._21;
	tempFrustumPlane[1].z = viewMat._34 - viewMat._31;
	tempFrustumPlane[1].w = viewMat._44 - viewMat._41;

	// Top Frustum Plane
	// Subtract second column of matrix from the fourth column
	tempFrustumPlane[2].x = viewMat._14 - viewMat._12;
	tempFrustumPlane[2].y = viewMat._24 - viewMat._22;
	tempFrustumPlane[2].z = viewMat._34 - viewMat._32;
	tempFrustumPlane[2].w = viewMat._44 - viewMat._42;

	// Bottom Frustum Plane
	// Add second column of the matrix to the fourth column
	tempFrustumPlane[3].x = viewMat._14 + viewMat._12;
	tempFrustumPlane[3].y = viewMat._24 + viewMat._22;
	tempFrustumPlane[3].z = viewMat._34 + viewMat._32;
	tempFrustumPlane[3].w = viewMat._44 + viewMat._42;

	// Near Frustum Plane
	// We could add the third column to the fourth column to get the near plane,
	// but we don't have to do this because the third column IS the near plane
	tempFrustumPlane[4].x = viewMat._13;
	tempFrustumPlane[4].y = viewMat._23;
	tempFrustumPlane[4].z = viewMat._33;
	tempFrustumPlane[4].w = viewMat._43;

	// Far Frustum Plane
	// Subtract third column of matrix from the fourth column
	tempFrustumPlane[5].x = viewMat._14 - viewMat._13;
	tempFrustumPlane[5].y = viewMat._24 - viewMat._23;
	tempFrustumPlane[5].z = viewMat._34 - viewMat._33;
	tempFrustumPlane[5].w = viewMat._44 - viewMat._43;

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
	mViewMatrix = XMMatrixLookAtLH(camPos, camTarget, camUp);
}