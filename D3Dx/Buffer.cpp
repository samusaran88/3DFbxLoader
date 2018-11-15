#include "stdafx.h"
#include "Buffer.h"


Buffer::Buffer()
	: vertexBuffer(NULL)
	, indexBuffer(NULL)
{
}


Buffer::~Buffer()
{
	if (vertexBuffer) vertexBuffer->Release();
	if (indexBuffer) indexBuffer->Release();
}

void Buffer::CreateBuffer(void* vertexData, UINT vertexDataSize, UINT vertexDataCount, void* indexData, UINT indexDataSize, UINT indexDataCount)
{
	D3D11_BUFFER_DESC bd = {};
	D3D11_SUBRESOURCE_DATA	initData = {};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = vertexDataSize * vertexDataCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	initData.pSysMem = vertexData;
	DEVICE->CreateBuffer(&bd, &initData, &vertexBuffer);

	UINT stride = vertexDataSize;
	UINT offset = 0;
	CONTEXT->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = indexDataSize * indexDataCount;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	initData.pSysMem = indexData;

	DEVICE->CreateBuffer(&bd, &initData, &indexBuffer);

	CONTEXT->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
}

void Buffer::CreateVertexBuffer(void * data, UINT dataSize, UINT dataCount)
{
	D3D11_BUFFER_DESC bd = {};
	D3D11_SUBRESOURCE_DATA	initData = {};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = dataSize * dataCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	initData.pSysMem = data;
	DEVICE->CreateBuffer(&bd, &initData, &vertexBuffer);

	UINT stride = dataSize;
	UINT offset = 0;
	//CONTEXT->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
}

void Buffer::CreateIndexBuffer(void * data, UINT dataSize, UINT dataCount)
{
	D3D11_BUFFER_DESC bd = {};
	D3D11_SUBRESOURCE_DATA	initData = {};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = dataSize * dataCount;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	initData.pSysMem = data;

	DEVICE->CreateBuffer(&bd, &initData, &indexBuffer);

	CONTEXT->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
}
