#pragma once

#include <d3d11_1.h>
#include <unordered_map>
#include <memory>

using namespace std;

static class InputLayoutManager
{
public:
	InputLayoutManager();
	~InputLayoutManager();

	static HRESULT GenerateInputLayout(string name, vector<string> layoutSetup, ID3D11Device* d3dDevice);

	static unordered_map<string, vector<D3D11_INPUT_ELEMENT_DESC>> _pInputLayoutList;
};
