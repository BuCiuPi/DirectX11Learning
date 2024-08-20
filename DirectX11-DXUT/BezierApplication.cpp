#include "BezierApplication.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "D3DUtil.h"
#include "DDSTextureLoader.h"

BezierApplication::BezierApplication(HINSTANCE hInstance) : DirectX11Application(hInstance)
{
	mRadius = 20.0f;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mLandWorld, I);
	g_View, I;
	g_World = I;

	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mLandMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLandMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
}

bool BezierApplication::Init(int nShowCmd)
{
	if (!D3DApp::Init(nShowCmd))
	{
		return false;
	}
	RenderStates::InitAll(g_pd3dDevice);

	BuildGeometryBuffer();
	BuildConstantBuffer();
	BuildFX();

	return true;
}

void BezierApplication::DrawScene()
{
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	//g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	WavePerFrameBuffer pfb;
	for (size_t i = 0; i < 3; i++)
	{
		pfb.gDirLights[i] = mDirLights[i];
	}
	XMStoreFloat4(&pfb.gEyePosW, mCamera.Position);
	g_pImmediateContext->UpdateSubresource(mPerFrameBuffer, 0, nullptr, &pfb, 0, 0);
	g_pImmediateContext->HSSetConstantBuffers(1, 1, &mPerFrameBuffer);
	g_pImmediateContext->PSSetConstantBuffers(1, 1, &mPerFrameBuffer);

	WaveConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(g_View);
	cb.mProjection = XMMatrixTranspose(g_Projection);
	XMVECTOR detBox = XMMatrixDeterminant(g_World);
	cb.mWorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(&detBox, g_World));

	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->HSSetShader(mHullShader, nullptr, 0);
	g_pImmediateContext->DSSetShader(mDomainShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

	g_pImmediateContext->PSSetSamplers(0, 1, &mSamplerLinear);

	//
	 //Draw the land.
	//
	g_pImmediateContext->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);

	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mLandWorld));
	cb.gTexTransform = XMMatrixTranspose(XMLoadFloat4x4(&mGrassTexTransform));
	cb.gMaterial = mLandMat;
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->HSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->DSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->PSSetShaderResources(0, 1, &mGrassMapSRV);


	g_pImmediateContext->RSSetState(RenderStates::WireframeRS);
	g_pImmediateContext->Draw(mVertexCount, 0);

	g_pSwapChain->Present(0, 0);
}

void BezierApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);

	static float t = 0.0f;
	if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static ULONGLONG timeStart = 0;
		ULONGLONG timeCur = GetTickCount64();
		if (timeStart == 0)
			timeStart = timeCur;
		t = (timeCur - timeStart) / 1000.0f;
	}
}

void BezierApplication::BuildGeometryBuffer()
{
	mVertexCount = 16;

	std::vector<SimpleVertex> vertices(mVertexCount);
	vertices[0].Pos = XMFLOAT3(-10.0f, -10.0f, +15.0f);
	vertices[1].Pos = XMFLOAT3(-5.0f, 0.0f, +15.0f);
	vertices[2].Pos = XMFLOAT3(+5.0f, 0.0f, +15.0f);
	vertices[3].Pos = XMFLOAT3(+10.0f, 0.0f, +15.0f);

	vertices[4].Pos = XMFLOAT3(-15.0f, 0.0f, +5.0f);
	vertices[5].Pos = XMFLOAT3(-5.0f, 0.0f, +5.0f);
	vertices[6].Pos = XMFLOAT3(+5.0f, 20.0f, +5.0f);
	vertices[7].Pos = XMFLOAT3(+15.0f, 0.0f, +5.0f);

	vertices[8].Pos = XMFLOAT3(-15.0f, 0.0f, -5.0f);
	vertices[9].Pos = XMFLOAT3(-5.0f, 0.0f, -5.0f);
	vertices[10].Pos = XMFLOAT3(+5.0f, 0.0f, -5.0f);
	vertices[11].Pos = XMFLOAT3(+15.0f, 0.0f, -5.0f);

	vertices[12].Pos = XMFLOAT3(-10.0f, 10.0f, -15.0f);
	vertices[13].Pos = XMFLOAT3(-5.0f, 0.0f, -15.0f);
	vertices[14].Pos = XMFLOAT3(+5.0f, 0.0f, -15.0f);
	vertices[15].Pos = XMFLOAT3(+25.0f, 10.0f, -15.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * mVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(g_pd3dDevice->CreateBuffer(&vbd, &vinitData, &mLandVB));
}

void BezierApplication::BuildConstantBuffer()
{
	D3D11_BUFFER_DESC bd;
	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WaveConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	HR(g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer));

	D3D11_BUFFER_DESC pfb;

	pfb.Usage = D3D11_USAGE_DEFAULT;
	pfb.ByteWidth = sizeof(PerFrameBuffer);
	pfb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pfb.CPUAccessFlags = 0;
	pfb.MiscFlags = 0;
	pfb.StructureByteStride = 0;
	HR(g_pd3dDevice->CreateBuffer(&pfb, nullptr, &mPerFrameBuffer));

	HR(CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/grass.dds", nullptr, &mGrassMapSRV));

	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(g_pd3dDevice->CreateSamplerState(&sampDesc, &mSamplerLinear));
}

void BezierApplication::CleanupDevice()
{
	ReleaseCOM(mLandVB);
	ReleaseCOM(mGrassMapSRV);

	RenderStates::DestroyAll();
}

void BezierApplication::BuildFX()
{
	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"BezierTessellation.fxh", "VS", "vs_5_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return;
	}

	InputLayouts::BuildVertexLayout(g_pd3dDevice, pVSBlob, InputLayoutDesc::Basic32, ARRAYSIZE(InputLayoutDesc::Basic32), &g_pVertexLayout);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return;
	}

	// Compile the hull shader
	ID3DBlob* pHSBlob = nullptr;
	hr = CompileShaderFromFile(L"BezierTessellation.fxh", "HS", "hs_5_0", &pHSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the Hull shader
	hr = g_pd3dDevice->CreateHullShader(pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), nullptr, &mHullShader);
	pHSBlob->Release();
	if (FAILED(hr))
		return;

	// Compile the Domain shader
	ID3DBlob* pDSBlob = nullptr;
	hr = CompileShaderFromFile(L"BezierTessellation.fxh", "DS", "ds_5_0", &pDSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the Domain shader
	hr = g_pd3dDevice->CreateDomainShader(pDSBlob->GetBufferPointer(), pDSBlob->GetBufferSize(), nullptr, &mDomainShader);
	pDSBlob->Release();
	if (FAILED(hr))
		return;

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"BezierTessellation.fxh", "PS", "ps_5_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return;
}
