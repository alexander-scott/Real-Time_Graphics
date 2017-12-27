//***************************************************************************************
// Camera.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Simple first person style camera class that lets the viewer explore the 3D scene.
//   -It keeps track of the camera coordinate system relative to the world space
//    so that the view matrix can be constructed.  
//   -It keeps track of the viewing frustum of the camera so that the projection
//    matrix can be obtained.
//***************************************************************************************

#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <sstream>
#include "resource.h"
#include <iostream>

#include <vector>
#include <memory>

#include "DX11AppHelper.h"

using namespace DirectX;

class SceneCamera
{
public:
	SceneCamera(float nearDepth, float farDepth, float windowWidth, float windowHeight, bool canFly);
	~SceneCamera();

	// Get/Set world camera position.
	XMVECTOR GetPosition()const { return XMLoadFloat3(&mCameraPos);	}
	XMFLOAT3 GetPosition3f()const { return mCameraPos; }
	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& v);

	XMVECTOR GetRightDirection() { return camRight; }
	XMVECTOR GetUpDirection() { return camForward; }
	XMVECTOR GetForwardDirection() { return camForward; }

	XMMATRIX GetViewMatrix()const { return mViewMatrix; }
	XMMATRIX GetProjectionMatrix()const { return mProjectionMatrix; }

	BoundingFrustum GetBoundingFrustum();

	std::vector<XMFLOAT4> GetFrustumPlanes();

	void OnMouseMove(int x, int y);

	void UpdateCameraView();

private:
	// Set frustum.
	void CreateProjectionMatrix();

	float mCameraPitch = 0;
	float mCameraYaw = 0;

	float moveLeftRight = 0.0f;
	float moveBackForward = 0.0f;

	POINT mLastMousePos;

	// Camera coordinate system with coordinates relative to world space.
	XMFLOAT3 mCameraPos = { 0.0f, 0.0f, 0.0f };

	// Direction vectors
	XMVECTOR camRight;
	XMVECTOR camForward;
	XMVECTOR camUp;

	// Cache frustum properties.
	float mNearZ = 0.0f;
	float mFarZ = 0.0f;
	float mAspect = 0.0f;
	float mFovY = 0.0f;
	float mNearWindowHeight = 0.0f;
	float mFarWindowHeight = 0.0f;

	bool mCanFly;

	// Cache View/Proj matrices.
	XMMATRIX mViewMatrix = Identity4x4();
	XMMATRIX mProjectionMatrix = Identity4x4();

	XMMATRIX Identity4x4()
	{
		static XMMATRIX I(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		return I;
	}
};