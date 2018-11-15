#include "stdafx.h"
#include "World.h"


World::World()
	: pos(0, 0, 0)
	, rot(0, 0, 0)
	, scale(1, 1, 1)
	, pivot(0, 0, 0)
	, matWorld(XMMatrixIdentity())
	, S(XMMatrixIdentity())
	, R(XMMatrixIdentity())
	, T(XMMatrixIdentity())
{
}


World::~World()
{
}

void World::Update()
{
	XMMATRIX matPivot, invMatPivot;
	matPivot = XMMatrixTranslation(pivot.x, pivot.y, pivot.z);
	invMatPivot = XMMatrixInverse(NULL, matPivot);

	XMMATRIX matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	S = invMatPivot * matScale * matPivot;

	XMMATRIX matRot = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
	R = invMatPivot * matRot * matPivot;

	T = XMMatrixTranslation(pos.x, pos.y, pos.z);

	matWorld = S * R * T;
}
