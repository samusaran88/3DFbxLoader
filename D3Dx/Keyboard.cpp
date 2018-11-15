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

	//한번 눌러졌을 때 1, 다시 누르면 0 토글 형태로
	//128 - 0일때 눌러져 있는 상태
	//129 - 1일때 눌러져 있는 상태
	GetKeyboardState(keyState);

	for (int i = 0; i < MAXKEY; ++i)
	{
		//0x80 = 128
		//128 & 128 = 128, 129 & 128 = 128, 0 & 128 = 0, 1 & 128 = 0
		byte key = keyState[i] & 0x80;	//눌러져 있으면 128, 안 눌러져 있으면 0 만들어
		keyState[i] = key ? 1 : 0;		//0이 아니면 다 1로 만들어

		int oldState = keyOldState[i];
		int state = keyState[i];

		if (oldState == 0 && state == 1) keyMap[i] = DOWN;
		else if (oldState == 1 && state == 0) keyMap[i] = UP;
		else if (oldState == 1 && state == 1) keyMap[i] = PRESS;
		else keyMap[i] = NONE;
	}
}
