#include "stdafx.h"
#include "Player.h"


Player::Player()
	: cube(NULL)
{
	cube = new Cube;
	cube->Create(XMFLOAT3(0, -10, 0), XMFLOAT3(1, 1, 1));
}


Player::~Player()
{
}

void Player::Update()
{
}

void Player::Render()
{

}
