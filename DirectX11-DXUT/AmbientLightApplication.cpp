#include "AmbientLightApplication.h"

#include <fstream>
#include <ratio>

#include "GameObject.h"
#include "GeometryGenerator.h"

AmbientLightApplication::AmbientLightApplication(HINSTANCE hinstance) : DirectX11Application(hinstance)
{
	mCamera.SetPosition(0.0f, 2.0f, -15.0f);
	mCamera.CameraSpeed = 50.0f;

}

bool AmbientLightApplication::Init(int nShowCmd)
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

void AmbientLightApplication::DrawScene()
{
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	if (GetAsyncKeyState('1') & 1)
	{
		mIsWireFrame = !mIsWireFrame;
	}

	if (mIsWireFrame)
	{
		g_pImmediateContext->RSSetState(RenderStates::WireframeRS);
	}
	g_pImmediateContext->IASetInputLayout(InputLayouts::NanoSuit);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mAmbientLightConstantBuffer.data.AmbientDown = XMFLOAT3(0.1f, 0.5f, 0.1f);
	mAmbientLightConstantBuffer.data.AmbientRange = XMFLOAT3(0.1f, 0.2f, 0.5f);

	mAmbientLightConstantBuffer.ApplyChanges();
	mAmbientLightConstantBuffer.PSShaderUpdate(1);

	g_pImmediateContext->VSSetShader(mNanoSuitVertexShader, 0, 0);
	g_pImmediateContext->PSSetShader(mNanoSuitPixelShader, 0, 0);
	g_pImmediateContext->PSSetSamplers(0, 1, &mSamplerLinear);

	mNanoSuitGameObject->Draw(mCamera.ViewProj());
	//draw sky

	mSky->Draw(g_pImmediateContext, mCamera);

	g_pSwapChain->Present(0, 0);
}

void AmbientLightApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);
}

void AmbientLightApplication::BuildGeometryBuffer()
{
}


void AmbientLightApplication::BuildConstantBuffer()
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


	//mSky = new Sky(g_pd3dDevice, L"Textures/grasscube1024.dds", 10.0f);
	//mSky = new Sky(g_pd3dDevice, L"Textures/snowcube1024.dds", 10.0f);
	mSky = new Sky(g_pd3dDevice, L"Textures/desertcube1024.dds", 10.0f);

	HRESULT hr = cb_vs_vertexshader.Initialize(g_pd3dDevice, g_pImmediateContext);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The Constant Buffer cannot be Initialized.", L"Error", MB_OK);
		return;
	}

	hr = mAmbientLightConstantBuffer.Initialize(g_pd3dDevice, g_pImmediateContext);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The Constant Buffer cannot be Initialized.", L"Error", MB_OK);
		return;
	}

	mNanoSuitGameObject = new GameObject();
	//mNanoSuitGameObject->Initialize("Models/Objects/nanosuit/nanosuit.obj", g_pd3dDevice, g_pImmediateContext, &cb_vs_vertexshader);
	mNanoSuitGameObject->Initialize("Models/Objects/nile/source/nile2.obj", g_pd3dDevice, g_pImmediateContext, &cb_vs_vertexshader);
}

void AmbientLightApplication::CleanupDevice()
{
	DirectX11Application::CleanupDevice();
	SafeDelete(mSky);
}

void AmbientLightApplication::BuildFX()
{
	mSky->BuildSkyFX(g_pd3dDevice);
	BuildNanoSuitFX();
}

void AmbientLightApplication::BuildNanoSuitFX()
{
	// Compile the vertex shader
	ID3DBlob* skyVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"AmbientLight.hlsl", "VS", "vs_5_0", &skyVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(skyVSBlob->GetBufferPointer(), skyVSBlob->GetBufferSize(), nullptr, &mNanoSuitVertexShader);
	if (FAILED(hr))
	{
		skyVSBlob->Release();
		return;
	}

	InputLayouts::BuildVertexLayout(g_pd3dDevice, skyVSBlob, InputLayoutDesc::NanoSuit, ARRAYSIZE(InputLayoutDesc::NanoSuit), &InputLayouts::NanoSuit);
	if (FAILED(hr))
	{
		skyVSBlob->Release();
		return;
	}

	// Compile the pixel shader
	ID3DBlob* skyPSBlob = nullptr;
	hr = CompileShaderFromFile(L"AmbientLight.hlsl", "PS", "ps_5_0", &skyPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(skyPSBlob->GetBufferPointer(), skyPSBlob->GetBufferSize(), nullptr, &mNanoSuitPixelShader);
	skyPSBlob->Release();
	if (FAILED(hr))
		return;
}
