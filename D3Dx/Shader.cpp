#include "stdafx.h"
#include "Shader.h"


Shader::Shader()
	: vertexShader(NULL)
	, pixelShader(NULL)
	, inputLayout(NULL)
	, vertexBuffer(NULL)
	, indexBuffer(NULL)
	, constantBuffer(NULL)
	, vsBlob(NULL)
	, psBlob(NULL)
	, stride(0)
	, offset(0)
{
}


Shader::~Shader()
{
	vertexShader->Release();
	pixelShader->Release();
	inputLayout->Release();

	vsBlob->Release();
	psBlob->Release();

	SAFE_RELEASE(vertexBuffer);	 
	SAFE_RELEASE(indexBuffer);	 
	SAFE_RELEASE(constantBuffer);
}

void Shader::CreateShader(wstring fileName)
{
	CompileShaderFromFile(fileName, "VertexMain", "vs_5_0", &vsBlob);
	CompileShaderFromFile(fileName, "PixelMain", "ps_5_0", &psBlob);

	D3DReflect
	(
		vsBlob->GetBufferPointer()
		, vsBlob->GetBufferSize()
		, IID_ID3D11ShaderReflection
		, (void**)&reflection
	);

	D3D11_SHADER_DESC shaderDesc;
	reflection->GetDesc(&shaderDesc);

	for (UINT i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		reflection->GetInputParameterDesc(i, &paramDesc);

		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		string temp = paramDesc.SemanticName;
		if (temp == "POSITION")
			elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;

		inputLayoutDesc.push_back(elementDesc);
	}

	DEVICE->CreateInputLayout
	(
		&inputLayoutDesc[0]
		, inputLayoutDesc.size()
		, vsBlob->GetBufferPointer()
		, vsBlob->GetBufferSize()
		, &inputLayout		
	);

	DEVICE->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &vertexShader);
	DEVICE->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &pixelShader);
}

void Shader::CreateBuffer(void* vertexData, UINT vertexDataSize, UINT vertexDataCount, void* indexData, UINT indexDataSize, UINT indexDataCount)
{
	D3D11_BUFFER_DESC		bd = {};
	D3D11_SUBRESOURCE_DATA	initData = {};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = vertexDataSize * vertexDataCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	initData.pSysMem = vertexData;
	DEVICE->CreateBuffer(&bd, &initData, &vertexBuffer);

	stride = vertexDataSize;
	offset = 0;

	if (indexData)
	{
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = indexDataSize * indexDataCount;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		initData.pSysMem = indexData;

		DEVICE->CreateBuffer(&bd, &initData, &indexBuffer);
	}

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	DEVICE->CreateBuffer(&bd, NULL, &constantBuffer);
}

void Shader::CreateBuffer(void * vertexData, UINT vertexDataSize, UINT vertexDataCount, void * indexData, UINT indexDataSize, UINT indexDataCount, void * constantBufferType)
{
}

void Shader::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC bd = {};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	DEVICE->CreateBuffer(&bd, NULL, &constantBuffer);
}

void Shader::UpdateBuffers(XMMATRIX world)
{
	ConstantBuffer cb;
	XMStoreFloat4x4(&cb.world, XMMatrixTranspose(world));
	XMStoreFloat4x4(&cb.view ,XMMatrixTranspose(CAMERA->GetView()));
	XMStoreFloat4x4(&cb.projection ,XMMatrixTranspose(CAMERA->GetProjection()));
	cb.lightDir = XMFLOAT4(-1, -1, 1, 0);
	CONTEXT->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);

	CONTEXT->VSSetShader(vertexShader, NULL, 0);
	CONTEXT->VSSetConstantBuffers(0, 1, &constantBuffer);
	CONTEXT->PSSetConstantBuffers(0, 1, &constantBuffer);
	CONTEXT->PSSetShader(pixelShader, NULL, 0);
}

void Shader::CompileShaderFromFile(wstring fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* errorBlob;
	hr = D3DX11CompileFromFile(
		fileName.c_str(),	//컴파일 할 셰이더 소스 코드를 담고 있는 fx파일
		NULL, NULL, 
		entryPoint,			//셰이더 프로그램의 진입점. 즉, 셰이더 주 함수의 이름
		shaderModel,		//사용할 셰이더 버전을 뜻하는 문자열
		dwShaderFlags,		//셰이더 코드의 컴파일 방식에 영향을 미치는 플래그들을 지정
		0, 
		NULL,				//셰이더를 비동기적으로 컴파일하기 위한 고급 옵션
		blobOut,			//컴파일된 셰이더를 담은 ID3DBlob 구조체를 가리키는 포인터를 돌려준다.
		&errorBlob,			//컴파일 오류시 오류 메시지를 담은 문자열
		NULL);				//비동기 컴파일시 오류코드를 조회하는데 쓴다. 위에 NULL이면 이거도 NULL

	if (FAILED(hr))
	{
		if (errorBlob != NULL)
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		if (errorBlob) errorBlob->Release();
	}
	if (errorBlob) errorBlob->Release();
}

void Shader::RenderFbx(UINT indexCount)
{
	CONTEXT->IASetInputLayout(inputLayout);
	CONTEXT->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CONTEXT->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	CONTEXT->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	CONTEXT->VSSetShader(vertexShader, NULL, 0);
	CONTEXT->PSSetShader(pixelShader, NULL, 0);

	CONTEXT->DrawIndexed(indexCount, 0, 0);
}

void Shader::RenderIndex(XMMATRIX world, UINT indexCount)
{
	CONTEXT->IASetInputLayout(inputLayout);
	CONTEXT->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CONTEXT->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	CONTEXT->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	ConstantBuffer cb;
	XMStoreFloat4x4(&cb.world, XMMatrixTranspose(world));
	XMStoreFloat4x4(&cb.view, XMMatrixTranspose(CAMERA->GetView()));
	XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(CAMERA->GetProjection()));
	CONTEXT->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	CONTEXT->VSSetConstantBuffers(0, 1, &constantBuffer);

	CONTEXT->VSSetShader(vertexShader, NULL, 0);
	CONTEXT->PSSetShader(pixelShader, NULL, 0);
	CONTEXT->DrawIndexed(indexCount, 0, 0);      
}

void Shader::RenderVertex(XMMATRIX world, UINT vertexCount)
{
	CONTEXT->IASetInputLayout(inputLayout);
	CONTEXT->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	CONTEXT->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	ConstantBuffer cb;
	XMStoreFloat4x4(&cb.world, XMMatrixTranspose(world));
	XMStoreFloat4x4(&cb.view, XMMatrixTranspose(CAMERA->GetView()));
	XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(CAMERA->GetProjection()));
	CONTEXT->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	CONTEXT->VSSetConstantBuffers(0, 1, &constantBuffer);

	CONTEXT->VSSetShader(vertexShader, NULL, 0);
	CONTEXT->PSSetShader(pixelShader, NULL, 0);
	CONTEXT->Draw(vertexCount, 0);
}

void Shader::Render(XMMATRIX world, UINT count, D3D_PRIMITIVE_TOPOLOGY topology)
{
	CONTEXT->IASetInputLayout(inputLayout);
	CONTEXT->IASetPrimitiveTopology(topology);

	CONTEXT->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	if (indexBuffer) CONTEXT->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	ConstantBuffer cb;
	XMStoreFloat4x4(&cb.world, XMMatrixTranspose(world));
	XMStoreFloat4x4(&cb.view, XMMatrixTranspose(CAMERA->GetView()));
	XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(CAMERA->GetProjection()));
	cb.lightDir = XMFLOAT4(-1, -1, 1, 0);
	CONTEXT->UpdateSubresource(constantBuffer, 0, NULL, &cb, 0, 0);
	CONTEXT->VSSetConstantBuffers(0, 1, &constantBuffer);
	CONTEXT->PSSetConstantBuffers(0, 1, &constantBuffer);

	CONTEXT->VSSetShader(vertexShader, NULL, 0);
	CONTEXT->PSSetShader(pixelShader, NULL, 0);
	if (indexBuffer)
	{
		CONTEXT->DrawIndexed(count, 0, 0);
	}
	else
	{
		CONTEXT->Draw(count, 0);
	}
}
