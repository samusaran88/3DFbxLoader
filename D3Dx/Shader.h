#pragma once
class Shader
{
private:
	struct ConstantBuffer
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
		XMFLOAT4 lightDir;
	};

	ID3D11VertexShader*		vertexShader;
	ID3D11PixelShader*		pixelShader;
	ID3D11InputLayout*		inputLayout;
	vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;

	ID3D11Buffer*			vertexBuffer;
	ID3D11Buffer*			indexBuffer;
	ID3D11Buffer*			constantBuffer;

	ID3DBlob* vsBlob;
	ID3DBlob* psBlob;

	ID3D11ShaderReflection* reflection;

	UINT stride;
	UINT offset;
public:
	Shader();
	~Shader();

	ID3D11VertexShader* GetVertexShader() { return vertexShader; }
	ID3D11PixelShader* GetPixelShader() { return pixelShader; }
	ID3D11Buffer* GetConstantBuffer() { return constantBuffer; }
	ID3DBlob* GetVSBlob() { return vsBlob; }
	ID3DBlob* GetPSBlob() { return psBlob; }
	vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayoutDesc() { return inputLayoutDesc; }

	void CreateShader(wstring fileName);
	void CreateBuffer(void* vertexData, UINT vertexDataSize, UINT vertexDataCount, void* indexData, UINT indexDataSize, UINT indexDataCount);
	void CreateBuffer(void* vertexData, UINT vertexDataSize, UINT vertexDataCount, void* indexData, UINT indexDataSize, UINT indexDataCount, void* constantBufferType);
	void CreateConstantBuffer();
	void UpdateBuffers(XMMATRIX world);

	void CompileShaderFromFile(wstring fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blobOut);

	void RenderFbx(UINT indexCount);
	void RenderIndex(XMMATRIX world, UINT indexCount);
	void RenderVertex(XMMATRIX world, UINT vertexCount);
	void Render(XMMATRIX world, UINT count, D3D_PRIMITIVE_TOPOLOGY topology);
};

