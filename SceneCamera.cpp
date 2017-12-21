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

	mCameraForwardDir = XMFLOAT3(0, 0, -1);
	mCameraUpDir = XMFLOAT3(0, 1, 0);

	UpdateCameraView();
}

SceneCamera::~SceneCamera()
{
}

void SceneCamera::SetPosition(float x, float y, float z)
{
	mCameraPos = XMFLOAT3(x, y, z);
	mViewDirty = true;
}

void SceneCamera::SetPosition(const XMFLOAT3& v)
{
	mCameraPos = v;
	mViewDirty = true;
}

void SceneCamera::CreateProjectionMatrix()
{
	mNearWindowHeight = 2.0f * mNearZ * tanf(0.5f*mFovY);
	mFarWindowHeight = 2.0f * mFarZ * tanf(0.5f*mFovY);

	XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	XMStoreFloat4x4(&mProjectionMatrix, P);
}

void SceneCamera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&mCameraPos, pos);
	XMStoreFloat3(&mCameraForwardDir, L);
	XMStoreFloat3(&mCameraRightDir, R);
	XMStoreFloat3(&mCameraUpDir, U);

	mViewDirty = true;
}

void SceneCamera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);

	mViewDirty = true;
}

XMFLOAT4X4 SceneCamera::GetViewMatrix()const
{
	return mViewMatrix;
}

XMFLOAT4X4 SceneCamera::GetProjectionMatrix()const
{
	return mProjectionMatrix;
}

void SceneCamera::Strafe(float d)
{
	// mPosition += d*mRight

	XMVECTOR speed = XMVectorReplicate(d);
	XMVECTOR rightDir = XMLoadFloat3(&mCameraRightDir);
	XMVECTOR position = XMLoadFloat3(&mCameraPos);
	XMStoreFloat3(&mCameraPos, XMVectorMultiplyAdd(speed, rightDir, position));

	mViewDirty = true;
}

void SceneCamera::Walk(float d)
{
	// mPosition += d*mLook

	XMVECTOR speed = XMVectorReplicate(d);
	XMVECTOR forwardDir = XMLoadFloat3(&mCameraForwardDir);
	XMVECTOR position = XMLoadFloat3(&mCameraPos);
	XMStoreFloat3(&mCameraPos, XMVectorMultiplyAdd(speed, forwardDir, position));

	mViewDirty = true;
}

void SceneCamera::OnMouseMove(int x, int y)
{
	// If right mouse button pressed
	if (GetAsyncKeyState(MK_RBUTTON))
	{
		// Pitch
		mCameraPitch = mCameraPitch - XMConvertToRadians(kCameraLookSpeed*static_cast<float>(y - mLastMousePos.y));
		// Yaw
		mCameraYaw = mCameraYaw - XMConvertToRadians(kCameraLookSpeed*static_cast<float>(x - mLastMousePos.x));

		mCameraPitch = min(kCameraMaxPitch / 16, mCameraPitch);
		mCameraPitch = max(-kCameraMaxPitch * 2, mCameraPitch);

		if (mCameraYaw > kPI)
			mCameraYaw -= kPI * 2;
		else if (mCameraYaw <= -kPI)
			mCameraYaw += kPI * 2;

		if (GetAsyncKeyState(VK_UP))
		{
			Walk(kCameraMoveSpeed);
		}
		else if (GetAsyncKeyState(VK_DOWN))
		{
			Walk(-kCameraMoveSpeed);
		}

		if (GetAsyncKeyState(VK_LEFT))
		{
			Strafe(-kCameraMoveSpeed * 30);
		}
		else if (GetAsyncKeyState(VK_RIGHT))
		{
			Strafe(kCameraMoveSpeed * 30);
		}
	}

	XMVECTOR lookAt = XMLoadFloat3(&mCameraForwardDir);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, lookAt));
	XMStoreFloat3(&mCameraRightDir, right);

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void SceneCamera::UpdateCameraView()
{
	mCameraForwardDir.x = kCameraRadius*sinf(mCameraPitch)*cosf(mCameraYaw);
	mCameraForwardDir.z = kCameraRadius*sinf(mCameraPitch)*sinf(mCameraYaw);
	mCameraForwardDir.y = kCameraRadius*cosf(mCameraPitch);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(mCameraPos.x, mCameraPos.y, mCameraPos.z, 1.0f);
	XMVECTOR target = XMVectorSet(mCameraForwardDir.x, mCameraForwardDir.y, mCameraForwardDir.z, 1.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mViewMatrix, view);

	CreateProjectionMatrix();
}