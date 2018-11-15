#include "stdafx.h"
#include "D3D.h"


D3D::D3D()
	: device(NULL)
	, immediateContext(NULL)
	, swapChain(NULL)
	, depthStencilView(NULL)
	, depthStencil(NULL)
	, renderTargetView(NULL)
{
}


D3D::~D3D()
{
}

void D3D::Setup()
{
	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(hWnd, &rc);

	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);


	DXGI_SWAP_CHAIN_DESC sd = {};	
	sd.BufferCount = 1;		
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//버퍼의 용도를 서술하는 구조체
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;	//다중표본화를 위해 추출할 표본 개수와 품질 수준을 서술하는 구조체
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,				//pAdapter : 이 함수로 생성할 장치를 나타내는 디스플레이 어댑터를 지정한다.
			driverType,
			NULL,				//소프트웨어 구동기를 지정한다.
			createDeviceFlags,	//추가적인 장치 생성 플래그들을 지정한다.
			featureLevels,		//원소들의 순서가 곧 기능 수준들을 점검하는 순서
			numFeatureLevels,	//원소 개수
			D3D11_SDK_VERSION,	//항상 이거로 지정한다.
			&sd,
			&swapChain,			//생성된 교환 사슬 인터페이스를 돌려준다.
			&device,			//함수가 생성한 장치를 돌려준다.
			&featureLevel,		//featureLevel 배열에서 처음으로 지원되는 기능을 돌려준다.
			&immediateContext);	//생성된 장치 문맥을 돌려준다.

		if (SUCCEEDED(hr))
			break;
	}

	// Create a render target view
	ID3D11Texture2D* backBuffer = NULL;

	swapChain->GetBuffer(			//GetBuffer 메서드를 호출해서 교환사슬을 가리키는 포인터를 얻는다.
		0,							//얻고자 하는 후면 버퍼의 색인이다. (후면버퍼가 여러개 있는 경우 중요)
		__uuidof(ID3D11Texture2D),	//버퍼의 인터페이스 형식을 지정하는 것으로 일반적으로 항상 2차원 텍스쳐를 위한 ID3D11Texture2D가 쓰인다. 
		(LPVOID*)&backBuffer);		//후면 버퍼를 가리키는 포인터를 돌려준다.

	device->CreateRenderTargetView(
		backBuffer,			//랜더 대상으로 사용할 자원
		NULL,				//D3D11_RENDER_TARGET_VIEW_DESC 구조체를 가리키는 포인터
		&renderTargetView);

	backBuffer->Release();


	//깊이 스텐실 텍스쳐 생성
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;	//밉맵 수준의 개수 - 깊이 스텐실 버퍼를 위한 텍스쳐에는 밉맵 수준이 하나만 있으면 된다.
	descDepth.ArraySize = 1;	//텍스쳐 배열의 텍스쳐 개수
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	
	descDepth.SampleDesc.Count = 1;	//다중 표본 개수와 품질 수준
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;	
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;	
	descDepth.CPUAccessFlags = 0;	//CPU가 자원에 접근하는 방식을 결정하는 플래그들을 지정한다.
	descDepth.MiscFlags = 0;	//기타 플래그들로, 깊이 스텐실 버퍼에는 적용되지 않는다.
	device->CreateTexture2D(&descDepth, NULL, &depthStencil);

	//깊이 스텐실 뷰 생성
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};	//자원의 원소의 자료형식을 서술한다.
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(depthStencil, &descDSV, &depthStencilView);

	immediateContext->OMSetRenderTargets(1,	&renderTargetView, depthStencilView);		


	//Viewport : 장면을 그려 넣고자 하는 후면 버퍼의 부푼 직사각형 영역
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	immediateContext->RSSetViewports(
		1,		//묶을 뷰포트 갯수 
		&vp);	//뷰포트 배열을 가리키는 포인터
}

void D3D::Destroy()
{
	device->Release();
	immediateContext->Release();
	swapChain->Release();

	depthStencilView->Release();
	depthStencil->Release();

	renderTargetView->Release();
}

void D3D::BeginScene()
{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; 
	immediateContext->ClearRenderTargetView(DX->GetRenderTargetView(), ClearColor);
	immediateContext->ClearDepthStencilView(DX->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3D::EndScene()
{
	swapChain->Present(0, 0);
}
