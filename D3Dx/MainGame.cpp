#include "stdafx.h"
#include "MainGame.h"


MainGame::MainGame()
	: time(0.0f)
	, dwTimeStart(0)
{
	CAMERA->Setup(XMFLOAT3(0.0f, 1.0f, -5.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	//cube = new Cube;
	//cube->Create(XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	F4X4Identity(world1);
	F4X4Identity(world2);

	grid = new Grid;

	//texture = new Texture;

	FLT = new FbxLoadTest;
}


MainGame::~MainGame()
{
	SAFE_DELETE(cube);
	SAFE_DELETE(grid);
	SAFE_DELETE(texture);
	SAFE_DELETE(FLT);
}

void MainGame::Update()
{
	CAMERA->Update();

	DWORD dwTimeCur = GetTickCount();
	if (dwTimeStart == 0)
	{
		dwTimeStart = dwTimeCur;
	}
	time = (dwTimeCur - dwTimeStart) / 1000.0f;

	XMStoreFloat4x4(&world1, XMMatrixRotationY(time));

	XMMATRIX spin = XMMatrixRotationZ(-time);
	XMMATRIX orbit = XMMatrixRotationY(-time * 2.0f);
	XMMATRIX translate = XMMatrixTranslation(-4, 0, 0);
	XMMATRIX scale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

	XMStoreFloat4x4(&world2, scale * spin * translate * orbit);

	FLT->Update();
}

void MainGame::Render()
{
	//CONTEXT->PSSetShaderResources(0, 1, TEXTURE(L"Image/earth.png"));
	//CONTEXT->PSSetSamplers(0, 1, SAMPLER);
	//cube->SetWorld(world1);
	//cube->Render();
	//cube->SetWorld(world2);
	//cube->Render();
	
	grid->Render();

	//texture->Render();

	FLT->Render();
}
