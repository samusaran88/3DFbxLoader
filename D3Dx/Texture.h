#pragma once
class Texture
{
private:
	Shader*	shader;

	UINT size;
	vector<WORD> index;

	XMMATRIX world;
public:
	Texture();
	~Texture();

	void Update();
	void Render();
};

