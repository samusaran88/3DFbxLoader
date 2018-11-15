#pragma once
class World
{
public:
	XMFLOAT3 pos;
	XMFLOAT3 rot;
	XMFLOAT3 scale;

	XMFLOAT3 pivot;

	XMMATRIX S, R, T;
	XMMATRIX matWorld;

	World();
	~World();

	void Update();
};

