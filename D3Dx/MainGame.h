#pragma once

class MainGame
{
private:
	Cube* cube;
	XMFLOAT4X4 world1;
	XMFLOAT4X4 world2;

	Grid* grid;
	Texture* texture;

	FbxLoadTest* FLT;

	float time;
	DWORD dwTimeStart;
public:
	MainGame();
	~MainGame();

	void Update();
	void Render();
};

