#pragma once

#include "imgui.h"

#include "DirectXInstance.h"

static class GUIController
{
	struct VERTEX_CONSTANT_BUFFER
	{
		float mvp[4][4];
	};

public:
	GUIController();
	~GUIController();

	static void SetupGUI();
	static void UpdateGUI();
	static void ExitGUI();

private:
	static bool InitGUI(void* hwnd, ID3D11Device* device, ID3D11DeviceContext* device_context);
	static void RenderDrawLists(ImDrawData* draw_data);
	static void GUINewFrame();
	static bool CreateDeviceObjects();
	static void InvalidateDeviceObjects();
	static void CreateFontsTexture();

public:
	static bool BlurOn;
	static bool BlurWasOn;
	static int BlurEffectPasses;
	static float BlurIntensity;

	static bool WhiteLightOn;
	static bool RedLightOn;
	static bool GreenLightOn;
	static bool BlueLightOn;

	static int ControlledLight;

	static bool SelfShadingOn;

	static bool FlyingCameraEnabled;

	static int SceneLightingMode;

	static ImGuiIO& io;

private:
	static ID3D11SamplerState* _pFontSampler;

	static ID3DBlob* _pVSBlob;
	static ID3DBlob* _pPSBlob;
	static ID3D11VertexShader* _pVertexShader;
	static ID3D11PixelShader* _pPixelShader;
	static ID3D11InputLayout* _pInputLayout;
	static ID3D11Buffer* _pVertexConstantBuffer;
	static ID3D11BlendState* _pBlendState;
	static ID3D11RasterizerState* _pRasterizerState;
	static ID3D11DepthStencilState* _pDepthStencilState;
	static ID3D11ShaderResourceView* _pFontTextureView;
	static ID3D11Buffer* _pVB;
	static ID3D11Buffer* _pIB;

	static int _pVertexBufferSize;
	static int _pIndexBufferSize;

	static INT64 _pTime;
	static INT64 _pTicksPerSecond;
};

