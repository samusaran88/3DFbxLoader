#include "stdafx.h"
#include "Keyboard.h"


Keyboard::Keyboard()
{
	ZeroMemory(keyState, sizeof(keyState));
	ZeroMemory(keyOldState, sizeof(keyOldState));
	ZeroMemory(keyMap, sizeof(keyMap));
}


Keyboard::~Keyboard()
{
}

void Keyboard::Update()
{
	memcpy(keyOldState, keyState, sizeof(keyOldState));

	ZeroMemory(keyState, sizeof(keyState));
	ZeroMemory(keyMap, sizeof(keyMap));

	//�ѹ� �������� �� 1, �ٽ� ������ 0 ��� ���·�
	//128 - 0�϶� ������ �ִ� ����
	//129 - 1�϶� ������ �ִ� ����
	GetKeyboardState(keyState);

	for (int i = 0; i < MAXKEY; ++i)
	{
		//0x80 = 128
		//128 & 128 = 128, 129 & 128 = 128, 0 & 128 = 0, 1 & 128 = 0
		byte key = keyState[i] & 0x80;	//������ ������ 128, �� ������ ������ 0 �����
		keyState[i] = key ? 1 : 0;		//0�� �ƴϸ� �� 1�� �����

		int oldState = keyOldState[i];
		int state = keyState[i];

		if (oldState == 0 && state == 1) keyMap[i] = DOWN;
		else if (oldState == 1 && state == 0) keyMap[i] = UP;
		else if (oldState == 1 && state == 1) keyMap[i] = PRESS;
		else keyMap[i] = NONE;
	}
}
