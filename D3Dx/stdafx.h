// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 특정 포함 파일이 들어 있는
// 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#define NOMINMAX
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>


// 여기서 프로그램에 필요한 추가 헤더를 참조합니다.
#include "DxDefine.h"
#include "FBXDefine.h"
#include "CommonMacroFunctions.h"


#define SAFE_DELETE(p)	{ if (p) delete p; p = nullptr; }
#define SAFE_DELETE_ARRAY(p)	{ if (p) delete [] p; p = nullptr; }
#define SAFE_RELEASE(p)	{ if (p) p->Release(); p = nullptr; }
#define SAFE_DESTROY(p) { if (p) p->Destroy(); p = nullptr; }

using namespace std;

//Framework Header File
#include "D3D.h"
#include "Shader.h"
#include "Buffer.h"
#include "VertexLayout.h"
#include "Camera.h"
#include "Keyboard.h"
#include "TextureManager.h"
#include "FbxLoader.h"

//GameObject Header File
#include "Cube.h"
#include "Grid.h"
#include "Texture.h"
#include "FbxLoadTest.h"

//MainGame Header File
#include "MainGame.h"


extern HWND hWnd;

#define WINNAME		(LPTSTR)TEXT("D3DX11")
#define WINSTARTX	100			//윈도우 시작좌표 X
#define WINSTARTY	0			//윈도우 시작좌표 Y
#define WINSIZEX	1280.0f		//윈도우 가로크기
#define WINSIZEY	720.0f		//윈도우 세로크기
#define WINSTYLE	WS_CAPTION | WS_SYSMENU

#define DX D3D::GetInstance()
#define DEVICE D3D::GetInstance()->GetDevice()
#define CONTEXT D3D::GetInstance()->GetDeviceContext()
#define SWAPCHAIN D3D::GetInstance()->GetSwapChain()
#define CAMERA Camera::GetInstance()
#define KEYBOARD Keyboard::GetInstance()
#define TEXTURE(fileDir) (TextureManager::GetInstance()->GetTextureRV(fileDir))
#define SAMPLER TextureManager::GetInstance()->GetSampler()
