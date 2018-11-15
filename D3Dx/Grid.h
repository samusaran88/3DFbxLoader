#pragma once

#define GRID_MAX 10
#define GRID_INTERVAL 1.0f;

class Grid
{
private:
	vector<PC_Vertex> vertex;

	XMFLOAT4X4 world;
	UINT size;

	Shader* shader;
	Buffer* buffer;
public:
	Grid();
	~Grid();

	void Render();
};

