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
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//������ �뵵�� �����ϴ� ����ü
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;	//����ǥ��ȭ�� ���� ������ ǥ�� ������ ǰ�� ������ �����ϴ� ����ü
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,				//pAdapter : �� �Լ��� ������ ��ġ�� ��Ÿ���� ���÷��� ����͸� �����Ѵ�.
			driverType,
			NULL,				//����Ʈ���� �����⸦ �����Ѵ�.
			createDeviceFlags,	//�߰����� ��ġ ���� �÷��׵��� �����Ѵ�.
			featureLevels,		//���ҵ��� ������ �� ��� ���ص��� �����ϴ� ����
			numFeatureLevels,	//���� ����
			D3D11_SDK_VERSION,	//�׻� �̰ŷ� �����Ѵ�.
			&sd,
			&swapChain,			//������ ��ȯ �罽 �������̽��� �����ش�.
			&device,			//�Լ��� ������ ��ġ�� �����ش�.
			&featureLevel,		//featureLevel �迭���� ó������ �����Ǵ� ����� �����ش�.
			&immediateContext);	//������ ��ġ ������ �����ش�.

		if (SUCCEEDED(hr))
			break;
	}

	// Create a render target view
	ID3D11Texture2D* backBuffer = NULL;

	swapChain->GetBuffer(			//GetBuffer �޼��带 ȣ���ؼ� ��ȯ�罽�� ����Ű�� �����͸� ��´�.
		0,							//����� �ϴ� �ĸ� ������ �����̴�. (�ĸ���۰� ������ �ִ� ��� �߿�)
		__uuidof(ID3D11Texture2D),	//������ �������̽� ������ �����ϴ� ������ �Ϲ������� �׻� 2���� �ؽ��ĸ� ���� ID3D11Texture2D�� ���δ�. 
		(LPVOID*)&backBuffer);		//�ĸ� ���۸� ����Ű�� �����͸� �����ش�.

	device->CreateRenderTargetView(
		backBuffer,			//���� ������� ����� �ڿ�
		NULL,				//D3D11_RENDER_TARGET_VIEW_DESC ����ü�� ����Ű�� ������
		&renderTargetView);

	backBuffer->Release();


	//���� ���ٽ� �ؽ��� ����
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;	//�Ӹ� ������ ���� - ���� ���ٽ� ���۸� ���� �ؽ��Ŀ��� �Ӹ� ������ �ϳ��� ������ �ȴ�.
	descDepth.ArraySize = 1;	//�ؽ��� �迭�� �ؽ��� ����
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	
	descDepth.SampleDesc.Count = 1;	//���� ǥ�� ������ ǰ�� ����
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;	
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;	
	descDepth.CPUAccessFlags = 0;	//CPU�� �ڿ��� �����ϴ� ����� �����ϴ� �÷��׵��� �����Ѵ�.
	descDepth.MiscFlags = 0;	//��Ÿ �÷��׵��, ���� ���ٽ� ���ۿ��� ������� �ʴ´�.
	device->CreateTexture2D(&descDepth, NULL, &depthStencil);

	//���� ���ٽ� �� ����
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};	//�ڿ��� ������ �ڷ������� �����Ѵ�.
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(depthStencil, &descDSV, &depthStencilView);

	immediateContext->OMSetRenderTargets(1,	&renderTargetView, depthStencilView);		


	//Viewport : ����� �׷� �ְ��� �ϴ� �ĸ� ������ ��Ǭ ���簢�� ����
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	immediateContext->RSSetViewports(
		1,		//���� ����Ʈ ���� 
		&vp);	//����Ʈ �迭�� ����Ű�� ������
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
