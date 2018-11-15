#pragma once

#define MAXKEY 255

class Keyboard
{
private:
	enum State
	{
		NONE, DOWN, UP, PRESS
	};

	byte keyState[MAXKEY];
	byte keyOldState[MAXKEY];
	byte keyMap[MAXKEY];

	Keyboard();
	~Keyboard();
public:
	static Keyboard* GetInstance()
	{
		static Keyboard instance;
		return &instance;
	}

	void Update();

	bool KeyDown(DWORD key) { return keyMap[key] == DOWN; }
	bool KeyUp(DWORD key) { return keyMap[key] == UP; }
	bool KeyPress(DWORD key) { return keyMap[key] == PRESS; }
};