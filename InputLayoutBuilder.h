#pragma once

#include <d3d11_1.h>
#include <unordered_map>
#include <memory>

using namespace std;

class InputLayoutBuilder
{
public:
	InputLayoutBuilder();
	~InputLayoutBuilder();

	HRESULT BuildInputLayout(string name, vector<string> layoutSetup, ID3D11Device* d3dDevice);
	vector<D3D11_INPUT_ELEMENT_DESC> GetD3D11InputDescs(string s) { return mInputLayoutList[s]; }

private:
	unordered_map<string, vector<D3D11_INPUT_ELEMENT_DESC>> mInputLayoutList;
};
