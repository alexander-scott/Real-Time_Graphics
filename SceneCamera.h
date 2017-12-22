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
	SceneCamera(float nearDepth, float farDepth, float windowWidth, float windowHeight);
	~SceneCamera();

	// Get/Set world camera position.
	XMVECTOR GetPosition()const { return XMLoadFloat3(&mCameraPos);	}
	XMFLOAT3 GetPosition3f()const { return mCameraPos; }
	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& v);
	
	// Get camera basis vectors.
	XMFLOAT3 GetUp3f()const { return mCameraUpDir; }

	XMFLOAT4X4 GetViewMatrix()const;
	XMFLOAT4X4 GetProjectionMatrix()const;

	void OnMouseMove(int x, int y);

	void UpdateCameraView();

private:
	// Strafe/Walk the camera a distance d.
	void Strafe(float d);
	void Walk(float d);

	// Set frustum.
	void CreateProjectionMatrix();

	float mCameraPitch = 0;
	float mCameraYaw = 0;

	POINT mLastMousePos;

	// Camera coordinate system with coordinates relative to world space.
	XMFLOAT3 mCameraPos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 mCameraRightDir = { 1.0f, 0.0f, 0.0f };
	XMFLOAT3 mCameraUpDir = { 0.0f, 1.0f, 0.0f };
	XMFLOAT3 mCameraForwardDir = { 0.0f, 0.0f, 1.0f };

	// Cache frustum properties.
	float mNearZ = 0.0f;
	float mFarZ = 0.0f;
	float mAspect = 0.0f;
	float mFovY = 0.0f;
	float mNearWindowHeight = 0.0f;
	float mFarWindowHeight = 0.0f;

	// Cache View/Proj matrices.
	XMFLOAT4X4 mViewMatrix = Identity4x4();
	XMFLOAT4X4 mProjectionMatrix = Identity4x4();

	XMFLOAT4X4 Identity4x4()
	{
		static XMFLOAT4X4 I(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		return I;
	}
};