#include "InputLayoutManager.h"

unordered_map<string, vector<D3D11_INPUT_ELEMENT_DESC>> InputLayoutManager::_pInputLayoutList;

InputLayoutManager::InputLayoutManager()
{
}

InputLayoutManager::~InputLayoutManager()
{
}

HRESULT InputLayoutManager::GenerateInputLayout(string layoutName, vector<string> layoutSetup, ID3D11Device* d3dDevice)
{
	vector<D3D11_INPUT_ELEMENT_DESC> layoutVector;

	UINT bytesTotal = 0;

	for (int i = 0; i < layoutSetup.size(); i++)
	{
		if (layoutSetup.at(i) == "POSITION")
		{
			layoutVector.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, bytesTotal, D3D11_INPUT_PER_VERTEX_DATA, 0 });
			bytesTotal += 12;
		}
		else if (layoutSetup.at(i) == "NORMAL")
		{
			layoutVector.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, bytesTotal, D3D11_INPUT_PER_VERTEX_DATA, 0 });
			bytesTotal += 12;
		}
		else if (layoutSetup.at(i) == "TEXCOORD")
		{
			layoutVector.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, bytesTotal, D3D11_INPUT_PER_VERTEX_DATA, 0 });
			bytesTotal += 8;
		}
		else if (layoutSetup.at(i) == "TANGENT")
		{
			layoutVector.push_back({ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, bytesTotal, D3D11_INPUT_PER_VERTEX_DATA, 0 });
			bytesTotal += 12;
		}
	}

	_pInputLayoutList[layoutName] = layoutVector;

	return S_OK;
}