#pragma once

struct PC_Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;

	PC_Vertex() : Pos(0, 0, 0) ,Color(0, 0, 0, 0) {}
	PC_Vertex(float x, float y, float z, XMFLOAT4 color)
	{
		Pos = { x, y, z };
		Color = color;
	}

	static D3D11_INPUT_ELEMENT_DESC desc[];
	static UINT count;
};

struct PT_Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 uv;

	PT_Vertex() : Pos(0, 0, 0), uv(0, 0) {}
	PT_Vertex(float x, float y, float z, float u, float v)
	{
		Pos = { x, y, z };
		uv = { u, v };
	}

	static D3D11_INPUT_ELEMENT_DESC desc[];
	static UINT count;
};

struct PNT_Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 uv;
	XMFLOAT3 Normal;

	PNT_Vertex() : Pos(0, 0, 0), uv(0, 0), Normal(0, 0, 0) {}
	PNT_Vertex(XMFLOAT3 pos, XMFLOAT2 texuv, XMFLOAT3 normal)
	{
		Pos = pos;
		uv = texuv;
		Normal = normal;
	}

	static D3D11_INPUT_ELEMENT_DESC desc[];
	static UINT count;
};