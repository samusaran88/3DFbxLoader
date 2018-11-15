#include "stdafx.h"
#include "Grid.h"


Grid::Grid()
{
	XMFLOAT4 color = { 1, 1, 1, 1 };
	vertex;

	for (int i = -GRID_MAX; i <= GRID_MAX; ++i)
	{
		vertex.push_back(PC_Vertex(-GRID_MAX, 0, i, color));
		vertex.push_back(PC_Vertex(GRID_MAX, 0, i, color));
		vertex.push_back(PC_Vertex(i, 0, -GRID_MAX, color));
		vertex.push_back(PC_Vertex(i, 0, GRID_MAX, color));
	}

	size = vertex.size();
	shader = new Shader;
	shader->CreateShader(L"TestShader.hlsl");
	shader->CreateBuffer(&vertex[0], sizeof(PC_Vertex), size, NULL, 0, 0);


	//buffer = new Buffer;
	//buffer->CreateVertexBuffer(&vertex[0], sizeof(Vertex), size);
	//CONTEXT->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//shader->CreateConstantBuffer();

	F4X4Identity(world);
}


Grid::~Grid()
{
	delete shader;
	delete buffer;
}

void Grid::Render()
{
	shader->RenderVertex(XMM(world), size);
}
