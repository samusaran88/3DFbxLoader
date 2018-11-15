#pragma once
class Buffer
{
private:
	ID3D11Buffer*	vertexBuffer;
	ID3D11Buffer*	indexBuffer;
public:
	Buffer();
	~Buffer();

	void CreateBuffer(void* vertexData, UINT vertexDataSize, UINT vertexDataCount, void* indexData, UINT indexDataSize, UINT indexDataCount);
	void CreateVertexBuffer(void* data, UINT dataSize, UINT dataCount);
	void CreateIndexBuffer(void* data, UINT dataSize, UINT dataCount);
};

