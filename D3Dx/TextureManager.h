#pragma once
class TextureManager
{
private:
	map<wstring, ID3D11ShaderResourceView*> textureRVs;
	ID3D11SamplerState* sampler;

	TextureManager();
	~TextureManager();
public:
	static TextureManager* GetInstance()
	{
		static TextureManager textureManager;
		return &textureManager;
	}

	ID3D11ShaderResourceView* const* GetTextureRV(wstring fileDir);
	ID3D11SamplerState* const* GetSampler() { return &sampler; }
};

