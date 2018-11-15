#include "stdafx.h"
#include "Cube.h"


Cube::Cube()
	: shader(NULL)
	, size(0)
{
	F4X4Identity(world);
}


Cube::~Cube()
{
	delete shader;
}

void Cube::Create(XMFLOAT3 pos, XMFLOAT3 half)
{
	vector<PNT_Vertex> vertex;
	vector<WORD> index;

	Pos = pos;
	Half = half;
	XMVECTOR vPos = XMLoadFloat3(&Pos);

	XMVECTOR pX = XMVectorSet(1, 0, 0, 0);
	XMVECTOR mX = XMVectorSet(-1, 0, 0, 0);
	XMVECTOR pY = XMVectorSet(0, 1, 0, 0);
	XMVECTOR mY = XMVectorSet(0, -1, 0, 0);
	XMVECTOR pZ = XMVectorSet(0, 0, 1, 0);
	XMVECTOR mZ = XMVectorSet(0, 0, -1, 0);

	//front
	AddVertex(vertex, vPos + (mX * Half.x) + (mY * Half.y) + (mZ * Half.z), XMFLOAT2(0.251f, 0.666f));	//0
	AddVertex(vertex, vPos + (mX * Half.x) + (pY * Half.y) + (mZ * Half.z), XMFLOAT2(0.251f, 0.334f));	//1
	AddVertex(vertex, vPos + (pX * Half.x) + (pY * Half.y) + (mZ * Half.z), XMFLOAT2(0.500f, 0.334f));	//2
	AddVertex(vertex, vPos + (pX * Half.x) + (pY * Half.y) + (mZ * Half.z), XMFLOAT2(0.500f, 0.334f));	//2
	AddVertex(vertex, vPos + (pX * Half.x) + (mY * Half.y) + (mZ * Half.z), XMFLOAT2(0.500f, 0.666f));	//3
	AddVertex(vertex, vPos + (mX * Half.x) + (mY * Half.y) + (mZ * Half.z), XMFLOAT2(0.251f, 0.666f));	//0
	//top	  
	AddVertex(vertex, vPos + (mX * Half.x) + (pY * Half.y) + (pZ * Half.z), XMFLOAT2(0.251f, 0.000f));	//5
	AddVertex(vertex, vPos + (pX * Half.x) + (pY * Half.y) + (pZ * Half.z), XMFLOAT2(0.500f, 0.000f));	//4
	AddVertex(vertex, vPos + (pX * Half.x) + (pY * Half.y) + (mZ * Half.z), XMFLOAT2(0.500f, 0.333f));	//2
	AddVertex(vertex, vPos + (pX * Half.x) + (pY * Half.y) + (mZ * Half.z), XMFLOAT2(0.500f, 0.333f));	//2
	AddVertex(vertex, vPos + (mX * Half.x) + (pY * Half.y) + (mZ * Half.z), XMFLOAT2(0.251f, 0.333f));	//1
	AddVertex(vertex, vPos + (mX * Half.x) + (pY * Half.y) + (pZ * Half.z), XMFLOAT2(0.251f, 0.000f));	//5
	//right	  				
	AddVertex(vertex, vPos + (pX * Half.x) + (pY * Half.y) + (mZ * Half.z), XMFLOAT2(0.501f, 0.334f));	//2
	AddVertex(vertex, vPos + (pX * Half.x) + (pY * Half.y) + (pZ * Half.z), XMFLOAT2(0.750f, 0.334f));	//4
	AddVertex(vertex, vPos + (pX * Half.x) + (mY * Half.y) + (pZ * Half.z), XMFLOAT2(0.750f, 0.666f));	//7
	AddVertex(vertex, vPos + (pX * Half.x) + (mY * Half.y) + (pZ * Half.z), XMFLOAT2(0.750f, 0.666f));	//7
	AddVertex(vertex, vPos + (pX * Half.x) + (mY * Half.y) + (mZ * Half.z), XMFLOAT2(0.501f, 0.666f));	//3
	AddVertex(vertex, vPos + (pX * Half.x) + (pY * Half.y) + (mZ * Half.z), XMFLOAT2(0.501f, 0.334f));	//2
	//bottom  				
	AddVertex(vertex, vPos + (mX * Half.x) + (mY * Half.y) + (mZ * Half.z), XMFLOAT2(0.251f, 0.667f));	//0
	AddVertex(vertex, vPos + (pX * Half.x) + (mY * Half.y) + (mZ * Half.z), XMFLOAT2(0.500f, 0.667f));	//3
	AddVertex(vertex, vPos + (pX * Half.x) + (mY * Half.y) + (pZ * Half.z), XMFLOAT2(0.500f, 1.000f));	//7
	AddVertex(vertex, vPos + (pX * Half.x) + (mY * Half.y) + (pZ * Half.z), XMFLOAT2(0.500f, 1.000f));	//7
	AddVertex(vertex, vPos + (mX * Half.x) + (mY * Half.y) + (pZ * Half.z), XMFLOAT2(0.251f, 1.000f));	//6
	AddVertex(vertex, vPos + (mX * Half.x) + (mY * Half.y) + (mZ * Half.z), XMFLOAT2(0.251f, 0.667f));	//0
	//left	  				
	AddVertex(vertex, vPos + (mX * Half.x) + (pY * Half.y) + (pZ * Half.z), XMFLOAT2(0.000f, 0.334f));	//5
	AddVertex(vertex, vPos + (mX * Half.x) + (pY * Half.y) + (mZ * Half.z), XMFLOAT2(0.250f, 0.334f));	//1
	AddVertex(vertex, vPos + (mX * Half.x) + (mY * Half.y) + (mZ * Half.z), XMFLOAT2(0.250f, 0.666f));	//0
	AddVertex(vertex, vPos + (mX * Half.x) + (mY * Half.y) + (mZ * Half.z), XMFLOAT2(0.250f, 0.666f));	//0
	AddVertex(vertex, vPos + (mX * Half.x) + (mY * Half.y) + (pZ * Half.z), XMFLOAT2(0.000f, 0.666f));	//6
	AddVertex(vertex, vPos + (mX * Half.x) + (pY * Half.y) + (pZ * Half.z), XMFLOAT2(0.000f, 0.334f));	//5
	//back	  				
	AddVertex(vertex, vPos + (pX * Half.x) + (mY * Half.y) + (pZ * Half.z), XMFLOAT2(0.751f, 0.666f));	//7
	AddVertex(vertex, vPos + (pX * Half.x) + (pY * Half.y) + (pZ * Half.z), XMFLOAT2(0.751f, 0.334f));	//4
	AddVertex(vertex, vPos + (mX * Half.x) + (pY * Half.y) + (pZ * Half.z), XMFLOAT2(1.000f, 0.334f));	//5
	AddVertex(vertex, vPos + (mX * Half.x) + (pY * Half.y) + (pZ * Half.z), XMFLOAT2(1.000f, 0.334f));	//5
	AddVertex(vertex, vPos + (mX * Half.x) + (mY * Half.y) + (pZ * Half.z), XMFLOAT2(1.000f, 0.666f));	//6
	AddVertex(vertex, vPos + (pX * Half.x) + (mY * Half.y) + (pZ * Half.z), XMFLOAT2(0.751f, 0.666f));	//7
	AddNormalVector(vertex);

	for (int i = 0; i < vertex.size(); ++i)
	{
		index.push_back(i);
	}

	shader = new Shader;
	shader->CreateShader(L"LightShader.hlsl");
	shader->CreateBuffer(&vertex[0], sizeof(PNT_Vertex), vertex.size(), &index[0], sizeof(WORD), index.size());
	size = index.size();
}

void Cube::AddVertex(vector<PNT_Vertex>& vector, XMVECTOR vp, XMFLOAT2 vt)
{
	PNT_Vertex temp;
	XMStoreFloat3(&temp.Pos, vp);
	temp.uv = vt;
	vector.push_back(temp);
}

void Cube::AddNormalVector(vector<PNT_Vertex>& vector)
{
	XMFLOAT3 vn;
	XMVECTOR vp1, vp2, vp3, v1, v2;
	for (int i = 0; i < vector.size() / 3; ++i)
	{
		v1 = XMLoadFloat3(&vector[i * 3 + 2].Pos) - XMLoadFloat3(&vector[i * 3 + 1].Pos);
		v2 = XMLoadFloat3(&vector[i * 3].Pos) - XMLoadFloat3(&vector[i * 3 + 1].Pos);
		XMStoreFloat3(&vn, XMVector3Normalize(XMVector3Cross(v1, v2)));
		vector[i * 3].Normal = vn;
		vector[i * 3 + 1].Normal = vn;
		vector[i * 3 + 2].Normal = vn;
	}
}

void Cube::Update()
{
}

void Cube::Render()
{
	shader->Render(XMLoadFloat4x4(&world), size, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
