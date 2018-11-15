#include "stdafx.h"
#include "Texture.h"


Texture::Texture()
	: shader(NULL)
{
	vector<PT_Vertex> vertex;
	vector<WORD> index;
	vertex.push_back(PT_Vertex(-1, 1, 0, 0, 0));
	vertex.push_back(PT_Vertex(1, 1, 0, 1, 0));
	vertex.push_back(PT_Vertex(-1, -1, 0, 0, 1));
	vertex.push_back(PT_Vertex(-1, -1, 0, 0, 1));
	vertex.push_back(PT_Vertex(1, 1, 0, 1, 0));
	vertex.push_back(PT_Vertex(1, -1, 0, 1, 1));
	index = { 0, 1, 2, 3, 4, 5 };

	shader = new Shader;
	shader->CreateShader(L"TextureShader.hlsl");
	shader->CreateBuffer(&vertex[0], sizeof(PT_Vertex), vertex.size(), &index[0], sizeof(WORD), index.size());
	size = index.size();

	world = XMMatrixIdentity();
	world *= XMMatrixTranslation(0, 2, 0);
}


Texture::~Texture()
{
	delete shader;
}

void Texture::Update()
{
}

void Texture::Render()
{
	CONTEXT->PSSetShaderResources(0, 1, TEXTURE(L"Image/Luda.jpg"));
	CONTEXT->PSSetSamplers(0, 1, SAMPLER);

	shader->Render(world, size, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
