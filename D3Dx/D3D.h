#pragma once
class D3D
{
private:
	ID3D11Device*			device;					
	ID3D11DeviceContext*	immediateContext;	
	IDXGISwapChain*			swapChain;

	ID3D11RenderTargetView* renderTargetView;

	ID3D11DepthStencilView*	depthStencilView;
	ID3D11Texture2D*		depthStencil;

	D3D();
	~D3D();
public:
	static D3D* GetInstance()
	{
		static D3D instance;
		return &instance;
	}
	ID3D11Device* GetDevice() { return device; }
	ID3D11DeviceContext* GetDeviceContext() { return immediateContext; }
	IDXGISwapChain* GetSwapChain() { return swapChain; }

	ID3D11DepthStencilView* GetDepthStencilView() { return depthStencilView; }
	ID3D11RenderTargetView* GetRenderTargetView() { return renderTargetView; }

	void Setup();
	void Destroy();

	void BeginScene();
	void EndScene();
};

