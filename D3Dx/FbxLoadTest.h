#pragma once

class FbxLoadTest
{
private:
	FbxLoader* testObj;

public:
	FbxLoadTest();
	~FbxLoadTest();

	void Update();
	void Render();
};

