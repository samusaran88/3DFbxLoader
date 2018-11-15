#pragma once

class Cube
{
private:
	Shader*	shader;
	Buffer*	buffer;
	XMFLOAT4X4 world;
	XMFLOAT3 Pos;
	XMFLOAT3 Half;
	UINT size;
public:
	Cube();
	~Cube();

	void SetWorld(XMFLOAT4X4 matWorld) { world = matWorld; }

	void Create(XMFLOAT3 pos, XMFLOAT3 half);
	void AddVertex(vector<PNT_Vertex>& vector, XMVECTOR vp, XMFLOAT2 vt);
	void AddNormalVector(vector<PNT_Vertex>& vector);

	void Update();
	void Render();
};

