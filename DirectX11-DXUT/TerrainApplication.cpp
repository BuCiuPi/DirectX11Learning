#include "TerrainApplication.h"

#include <fstream>

#include "GeometryGenerator.h"
#include "Terrain.h"

TerrainApplication::TerrainApplication(HINSTANCE hinstance) : DirectX11Application(hinstance)
{
	mCamera.SetPosition(0.0f, 2.0f, -15.0f);
	mCamera.CameraSpeed = 50.0f;

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
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
}

bool TerrainApplication::Init(int nShowCmd)
{
	if (!D3DApp::Init(nShowCmd))
	{
		return false;
	}
	RenderStates::InitAll(g_pd3dDevice);

	Terrain::InitInfo tii;
	tii.HeightMapFileName = L"Textures/terrain.raw";
	tii.BlendMapFileName = L"Textures/blend.dds";

	tii.LayerFileNames[0] = L"Textures/grass.dds";
	tii.LayerFileNames[1] = L"Textures/darkdirt.dds";
	tii.LayerFileNames[2] = L"Textures/stone.dds";
	tii.LayerFileNames[3] = L"Textures/lightdirt.dds";
	tii.LayerFileNames[4] = L"Textures/snow.dds";

	tii.HeightScale = 50.0f;
	tii.HeightmapWidth = 2049;
	tii.HeightmapHeight = 2049;
	tii.CellSpacing = 0.5f;


	BuildGeometryBuffer();
	BuildConstantBuffer();
	BuildFX();

	mTerrain.Init(g_pd3dDevice, g_pImmediateContext, tii);

	return true;
}

void TerrainApplication::DrawScene()
{
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	g_pImmediateContext->IASetInputLayout(InputLayouts::Terrain);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	g_pImmediateContext->VSSetShader(mTerrainVertexShader, nullptr, 0);
	g_pImmediateContext->HSSetShader(mTerrainHullShader, nullptr, 0);
	g_pImmediateContext->DSSetShader(mTerrainDomainShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(mTerrainPixelShader, nullptr, 0);

	TerrainPerFrameBuffer pfb;
	for (size_t i = 0; i < 3; i++)
	{
		pfb.gDirLights[i] = mDirLights[i];
	}
	pfb.gEyePosW = mCamera.GetPosition();

	pfb.gMinDist = 20.0f;
	pfb.gMaxDist = 500.0f;

	pfb.gMinTex = 0.0f;
	pfb.gMaxTex = 6.0f;
	
	pfb.gTexelCellSpaceU = 1.0f / mTerrain.mInfo.HeightmapWidth;
	pfb.gTexelCellSpaceV = 1.0f / mTerrain.mInfo.HeightmapHeight;
	pfb.gWorldCellSpace = mTerrain.mInfo.CellSpacing;

	pfb.gTexScale = XMFLOAT2(50.0f, 50.0f);

	ExtractFrustumPlanes(pfb.gWorldFrustumPlanes, mCamera.ViewProj());


	g_pImmediateContext->UpdateSubresource(mTerrainPerFrameBuffer, 0, nullptr, &pfb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &mTerrainPerFrameBuffer);
	g_pImmediateContext->HSSetConstantBuffers(1, 1, &mTerrainPerFrameBuffer);
	g_pImmediateContext->DSSetConstantBuffers(1, 1, &mTerrainPerFrameBuffer);
	g_pImmediateContext->PSSetConstantBuffers(1, 1, &mTerrainPerFrameBuffer);


	TerrainConstantBuffer tcb;
	tcb.mWorld = XMMatrixTranspose(g_World);
	tcb.mView = XMMatrixTranspose(mCamera.View());
	tcb.mProjection = XMMatrixTranspose(mCamera.Proj());
	tcb.gMaterial = mTerrain.mMat;

	g_pImmediateContext->UpdateSubresource(mTerrainConstantBuffer, 0, nullptr, &tcb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &mTerrainConstantBuffer);
	g_pImmediateContext->DSSetConstantBuffers(0, 1, &mTerrainConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &mTerrainConstantBuffer);

	g_pImmediateContext->VSGetSamplers(0, 1, &mSamplerLinear);
	g_pImmediateContext->HSSetSamplers(0, 1, &mSamplerLinear);
	g_pImmediateContext->DSSetSamplers(0, 1, &mSamplerLinear);
	g_pImmediateContext->PSSetSamplers(0, 1, &mSamplerLinear);

	mTerrain.Draw(g_pImmediateContext);

	//draw sky

	g_pImmediateContext->VSSetShader(mSkyVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(mSkyPixelShader, nullptr, 0);

	ID3D11ShaderResourceView* skySRV = mSky->CubeMapSRV();
	g_pImmediateContext->PSSetShaderResources(1, 1, &skySRV);

	SkyConstantBuffer scb;
	XMFLOAT3 camPos = mCamera.GetPosition();
	XMMATRIX skyWorld = XMMatrixTranslation(camPos.x, camPos.y, camPos.z);

	scb.mMVP = XMMatrixTranspose(XMMatrixMultiply(skyWorld, mCamera.ViewProj()));

	g_pImmediateContext->UpdateSubresource(mSkyConstantBuffer, 0, nullptr, &scb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &mSkyConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &mSkyConstantBuffer);

	g_pImmediateContext->OMSetDepthStencilState(RenderStates::LessEqualDSS, 0);
	g_pImmediateContext->RSSetState(RenderStates::NoCullRS);

	mSky->Draw(g_pImmediateContext, mCamera);

	g_pImmediateContext->OMSetDepthStencilState(nullptr, 0);
	g_pImmediateContext->RSSetState(nullptr);

	g_pSwapChain->Present(0, 0);
}

void TerrainApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);
}

void TerrainApplication::BuildGeometryBuffer()
{
}


void TerrainApplication::BuildConstantBuffer()
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

	D3D11_BUFFER_DESC tbd;
	// Create the constant buffer
	tbd.Usage = D3D11_USAGE_DEFAULT;
	tbd.ByteWidth = sizeof(TerrainConstantBuffer);
	tbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tbd.CPUAccessFlags = 0;
	tbd.MiscFlags = 0;
	tbd.StructureByteStride = 0;

	HR(g_pd3dDevice->CreateBuffer(&tbd, nullptr, &mTerrainConstantBuffer));

	D3D11_BUFFER_DESC sbd;
	// Create the constant buffer
	sbd.Usage = D3D11_USAGE_DEFAULT;
	sbd.ByteWidth = sizeof(SkyConstantBuffer);
	sbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	sbd.CPUAccessFlags = 0;
	sbd.MiscFlags = 0;
	sbd.StructureByteStride = 0;

	HR(g_pd3dDevice->CreateBuffer(&sbd, nullptr, &mSkyConstantBuffer));

	D3D11_BUFFER_DESC pfb;

	pfb.Usage = D3D11_USAGE_DEFAULT;
	pfb.ByteWidth = sizeof(PerFrameBuffer);
	pfb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pfb.CPUAccessFlags = 0;
	pfb.MiscFlags = 0;
	pfb.StructureByteStride = 0;
	HR(g_pd3dDevice->CreateBuffer(&pfb, nullptr, &mPerFrameBuffer));

	D3D11_BUFFER_DESC tpfb;

	tpfb.Usage = D3D11_USAGE_DEFAULT;
	tpfb.ByteWidth = sizeof(TerrainPerFrameBuffer);
	tpfb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tpfb.CPUAccessFlags = 0;
	tpfb.MiscFlags = 0;
	tpfb.StructureByteStride = 0;
	HR(g_pd3dDevice->CreateBuffer(&tpfb, nullptr, &mTerrainPerFrameBuffer));


	//mSky = new Sky(g_pd3dDevice, L"Textures/grasscube1024.dds", 10.0f);
	//mSky = new Sky(g_pd3dDevice, L"Textures/snowcube1024.dds", 10.0f);
	mSky = new Sky(g_pd3dDevice, L"Textures/desertcube1024.dds", 10.0f);

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

void TerrainApplication::CleanupDevice()
{
	DirectX11Application::CleanupDevice();
	SafeDelete(mSky);
}

void TerrainApplication::BuildFX()
{
	////////////////////////////// Normal Displacement

	ID3DBlob* dispVSBlob = nullptr;
	HRESULT	hr = CompileShaderFromFile(L"Terrain.fxh", "VS", "vs_5_0", &dispVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(dispVSBlob->GetBufferPointer(), dispVSBlob->GetBufferSize(), nullptr, &mTerrainVertexShader);
	if (FAILED(hr))
	{
		dispVSBlob->Release();
		return;
	}

	InputLayouts::BuildVertexLayout(g_pd3dDevice, dispVSBlob, InputLayoutDesc::Terrain, ARRAYSIZE(InputLayoutDesc::Terrain), &InputLayouts::Terrain);
	if (FAILED(hr))
	{
		dispVSBlob->Release();
		return;
	}

	// Compile the hull shader
	ID3DBlob* pHSBlob = nullptr;
	hr = CompileShaderFromFile(L"Terrain.fxh", "HS", "hs_5_0", &pHSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the Hull shader
	hr = g_pd3dDevice->CreateHullShader(pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), nullptr, &mTerrainHullShader);
	pHSBlob->Release();
	if (FAILED(hr))
		return;

	// Compile the Domain shader
	ID3DBlob* pDSBlob = nullptr;
	hr = CompileShaderFromFile(L"Terrain.fxh", "DS", "ds_5_0", &pDSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the Domain shader
	hr = g_pd3dDevice->CreateDomainShader(pDSBlob->GetBufferPointer(), pDSBlob->GetBufferSize(), nullptr, &mTerrainDomainShader);
	pDSBlob->Release();
	if (FAILED(hr))
		return;

	// Compile the pixel shader
	ID3DBlob* dispPSBlob = nullptr;
	hr = CompileShaderFromFile(L"Terrain.fxh", "PS", "ps_5_0", &dispPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(dispPSBlob->GetBufferPointer(), dispPSBlob->GetBufferSize(), nullptr, &mTerrainPixelShader);
	dispPSBlob->Release();
	if (FAILED(hr))
		return;

	////////////////////////////// Sky

	// Compile the vertex shader
	ID3DBlob* skyVSBlob = nullptr;
	hr = CompileShaderFromFile(L"Sky.fxh", "VS", "vs_5_0", &skyVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(skyVSBlob->GetBufferPointer(), skyVSBlob->GetBufferSize(), nullptr, &mSkyVertexShader);
	if (FAILED(hr))
	{
		skyVSBlob->Release();
		return;
	}

	InputLayouts::BuildVertexLayout(g_pd3dDevice, skyVSBlob, InputLayoutDesc::Pos, ARRAYSIZE(InputLayoutDesc::Pos), &InputLayouts::Pos);
	if (FAILED(hr))
	{
		skyVSBlob->Release();
		return;
	}

	// Compile the pixel shader
	ID3DBlob* skyPSBlob = nullptr;
	hr = CompileShaderFromFile(L"Sky.fxh", "PS", "ps_5_0", &skyPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(skyPSBlob->GetBufferPointer(), skyPSBlob->GetBufferSize(), nullptr, &mSkyPixelShader);
	skyPSBlob->Release();
	if (FAILED(hr))
		return;
}
