#include "stdafx.h"
#include "TextureManager.h"


TextureManager::TextureManager()
{
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	DEVICE->CreateSamplerState(&sampDesc, &sampler);
}


TextureManager::~TextureManager()
{
	for (auto iter : textureRVs)
	{
		iter.second->Release();
	}
	SAFE_RELEASE(sampler);
}

ID3D11ShaderResourceView* const* TextureManager::GetTextureRV(wstring fileDir)
{
	if (textureRVs.find(fileDir) == textureRVs.end())
	{
		ID3D11ShaderResourceView* SRV;
		D3DX11CreateShaderResourceViewFromFile(DEVICE, fileDir.c_str(), NULL, NULL, &SRV, NULL);
		textureRVs.insert(make_pair(fileDir, SRV));
		return &textureRVs[fileDir];
	}
	else
	{
		return &textureRVs[fileDir];
	}
	return nullptr;
}
