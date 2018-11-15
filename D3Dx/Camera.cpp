#include "stdafx.h"
#include "Camera.h"


Camera::Camera()
	: RButtonDown(false)
	, RotY(0.0f)
	, RotX(0.0f)
	, Distance(10.0f)
{
}


Camera::~Camera()
{
}

void Camera::Setup(XMFLOAT3 eye, XMFLOAT3 lookAt, XMFLOAT3 up)
{
	Eye = eye;
	At = lookAt;
	Up = up;
	XMStoreFloat4x4(&view, XMMatrixLookAtLH(XMV(Eye), XMV(At), XMV(Up)));

	RECT rc;
	GetClientRect(hWnd, &rc);

	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 10000.0f));
}

void Camera::Update()
{
	XMStoreFloat3(&Eye ,XMVector3TransformNormal(XMVectorSet(0, 0, -Distance, 0), XMMatrixRotationRollPitchYaw(RotX, RotY, 0)) + XMV(At));

	XMStoreFloat4x4(&view ,XMMatrixLookAtLH(XMV(Eye), XMV(At), XMV(Up)));
}

void Camera::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_RBUTTONDOWN:
	{
		PrevMouse.x = LOWORD(lParam);
		PrevMouse.y = HIWORD(lParam);
		RButtonDown = true;
	}
	break;
	case WM_RBUTTONUP:
	{
		RButtonDown = false;
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (RButtonDown)
		{
			POINT ptCurrMouse;
			PtMouse.x = ptCurrMouse.x = LOWORD(lParam);
			PtMouse.y = ptCurrMouse.y = HIWORD(lParam);

			RotY += (ptCurrMouse.x - PrevMouse.x) / 100.0f;
			RotX += (ptCurrMouse.y - PrevMouse.y) / 100.0f;

			// X축 회전은 위아래 90도 제한한다.
			if (RotX <= -XM_PI * 0.5f)// + 0.0001f)
				RotX = -XM_PI * 0.5f +0.0001f;
			else if (RotX >= XM_PI * 0.5f)// - 0.0001f)
				RotX = XM_PI * 0.5f - 0.0001f;

			PrevMouse = ptCurrMouse;
		}
	}
	break;
	case WM_MOUSEWHEEL:
	{
		Distance -= GET_WHEEL_DELTA_WPARAM(wParam) / 100.0f;
	}
	break;
	}
}
