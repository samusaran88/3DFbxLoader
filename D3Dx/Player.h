#pragma once
class Player
{
private:
	Cube* cube;
public:
	Player();
	~Player();

	void Update();
	void Render();
};

