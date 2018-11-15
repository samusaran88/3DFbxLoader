#pragma once
class Camera
{
private:
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;

	XMFLOAT3 Eye;
	XMFLOAT3 At;
	XMFLOAT3 Up;

	POINT PtMouse;
	POINT PrevMouse;
	bool RButtonDown;
	float RotY;
	float RotX;
	float Distance;

	Camera();
	~Camera();
public:
	static Camera* GetInstance()
	{
		static Camera instance;
		return &instance;
	}

	XMMATRIX GetView() { return XMM(view); }
	XMMATRIX GetProjection() { return XMM(projection); }

	void SetCameraPosition(XMFLOAT3 pos) { Eye = pos; }
	void SetCameraFocus(XMFLOAT3 target) { At = target; }

	void Setup(XMFLOAT3 eye, XMFLOAT3 lookAt, XMFLOAT3 up);
	void Update();
	void WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};