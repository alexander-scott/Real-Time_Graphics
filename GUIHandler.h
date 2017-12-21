#pragma once

#include "imgui.h"

#include "DX11AppHelper.h"

static class GUIHandler
{
	struct VERTEX_CONSTANT_BUFFER
	{
		float mvp[4][4];
	};

public:
	GUIHandler();
	~GUIHandler();

	static void SetupGUI();
	static void UpdateGUI();
	static void ExitGUI();

	static void ResetBlurOptions();

private:
	static bool InitGUI(void* hwnd, ID3D11Device* device, ID3D11DeviceContext* device_context);
	static void RenderDrawLists(ImDrawData* draw_data);
	static void GUINewFrame();
	static bool CreateDeviceObjects();
	static void InvalidateDeviceObjects();
	static void CreateFontsTexture();

public:
	static int _pShaderControlOption;

	static bool _pBlurOn;
	static bool _pBlurWasOn;
	static int _pBlurEffectPasses;
	static float _pBlurIntensity;

	static bool _pShadowMappingOn;

	static bool _pWhiteLightOn;
	static bool _pRedLightOn;
	static bool _pGreenLightOn;
	static bool _pBlueLightOn;

	static int _pControlledLight;

	static bool _pDOFActive;

	static bool _pSelfShadingOn;

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

