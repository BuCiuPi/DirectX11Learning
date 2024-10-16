#include "LightingApplication.h"

#include <fstream>
#include <ratio>
#include <sstream>

#include "GameObject.h"
#include "GeometryGenerator.h"

LightingApplication::LightingApplication(HINSTANCE hinstance) : DirectX11Application(hinstance)
{
	mCamera.SetPosition(0.0f, 20.0f, -15.0f);
	mCamera.CameraSpeed = 50.0f;

	mLightingConstantBuffer.data.directionalLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mLightingConstantBuffer.data.directionalLight.Diffuse = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	mLightingConstantBuffer.data.directionalLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLightingConstantBuffer.data.directionalLight.Direction = XMFLOAT3(-0.57735f, 0.57735f, -0.57735f);

	mLightingConstantBuffer.data.pointLight[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mLightingConstantBuffer.data.pointLight[0].Diffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	mLightingConstantBuffer.data.pointLight[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLightingConstantBuffer.data.pointLight[0].Position = XMFLOAT3(0.0f, 15.0f, 0.0f);
	mLightingConstantBuffer.data.pointLight[0].Range = 40.0f;

	mLightingConstantBuffer.data.pointLight[1].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mLightingConstantBuffer.data.pointLight[1].Diffuse = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	mLightingConstantBuffer.data.pointLight[1].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLightingConstantBuffer.data.pointLight[1].Position = XMFLOAT3(10.0f, 20.0f, -10.0f);
	mLightingConstantBuffer.data.pointLight[1].Range = 40.0f;

	mLightingConstantBuffer.data.pointLight[2].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mLightingConstantBuffer.data.pointLight[2].Diffuse = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	mLightingConstantBuffer.data.pointLight[2].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLightingConstantBuffer.data.pointLight[2].Position = XMFLOAT3(-10.0f, 20.0f, 10.0f);
	mLightingConstantBuffer.data.pointLight[2].Range = 40.0f;

	mLightingConstantBuffer.data.spotLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mLightingConstantBuffer.data.spotLight.Diffuse = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
	mLightingConstantBuffer.data.spotLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLightingConstantBuffer.data.spotLight.Position = XMFLOAT3(0.0f, 20.0f, -5.0f);

	mLightingConstantBuffer.data.spotLight.Direction = XMFLOAT3(0.5f, -1.0f, 0.3f);
	float spotLightInner = 1.0f / cosf(XM_PI * 50.0f / 180.0f);
	float spotLightOuter = cosf(XM_PI * 55.0f / 180.0f);
	mLightingConstantBuffer.data.spotLight.Attenuation = XMFLOAT3(spotLightInner, spotLightOuter, 0.0f);
	mLightingConstantBuffer.data.spotLight.Range = 200.0f;

	mLightingConstantBuffer.data.capsuleLight.Diffuse = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
	mLightingConstantBuffer.data.capsuleLight.Position = XMFLOAT3(0.0f,10.0f, -5.0f);
	mLightingConstantBuffer.data.capsuleLight.Range = 40.0f;
	mLightingConstantBuffer.data.capsuleLight.Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);

	mLightingConstantBuffer.data.fourCapsuleLight.DiffuseR = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	mLightingConstantBuffer.data.fourCapsuleLight.DiffuseG = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	mLightingConstantBuffer.data.fourCapsuleLight.DiffuseB = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	mLightingConstantBuffer.data.fourCapsuleLight.PositionX = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mLightingConstantBuffer.data.fourCapsuleLight.PositionY = XMFLOAT4(30.0f, 30.0f, 30.0f, 30.0f);
	mLightingConstantBuffer.data.fourCapsuleLight.PositionZ = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	mLightingConstantBuffer.data.fourCapsuleLight.Range =  XMFLOAT4(40.0f, 40.0f, 40.0f, 40.0f);
	mLightingConstantBuffer.data.fourCapsuleLight.Range =  XMFLOAT4(80.0f, 80.0f, 80.0f, 80.0f);

	mLightingConstantBuffer.data.fourCapsuleLight.DirectionX = XMFLOAT4(1.0f, -1.0f, 0.0f, 0.0f);
	mLightingConstantBuffer.data.fourCapsuleLight.DirectionY = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mLightingConstantBuffer.data.fourCapsuleLight.DirectionZ = XMFLOAT4(0.0f, 0.0f, 1.0f, -1.0f);

	mLightingConstantBuffer.data.fourCapsuleLight.Len =  XMFLOAT4(10.0f, 10.0f, 10.0f, 10.0f);


	this->mMaterial.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	this->mMaterial.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	this->mMaterial.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
	this->mMaterial.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);


}

bool LightingApplication::Init(int nShowCmd)
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

void LightingApplication::DrawScene()
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

	// set constant buffer
	XMMATRIX worldMatrix = mNanoSuitGameObject->GetWorldMatrix();
	XMMATRIX MVP = worldMatrix * mCamera.ViewProj();
	this->cb_vs_vertexshader.data.gWorld = XMMatrixTranspose(worldMatrix);
	this->cb_vs_vertexshader.data.gWorldViewProj = XMMatrixTranspose(MVP);
	this->cb_vs_vertexshader.data.material = mMaterial;
	this->cb_vs_vertexshader.ApplyChanges();
	this->cb_vs_vertexshader.VSShaderUpdate(0);

	mLightingConstantBuffer.data.AmbientDown = XMFLOAT3(0.1f, 0.2f, 0.1f);
	mLightingConstantBuffer.data.AmbientRange = XMFLOAT3(0.1f, 0.2f, 0.2f);
	mLightingConstantBuffer.data.gEyePosition = mCamera.GetPosition();

	mLightingConstantBuffer.ApplyChanges();
	mLightingConstantBuffer.PSShaderUpdate(1);

	g_pImmediateContext->VSSetShader(mNanoSuitVertexShader, 0, 0);
	g_pImmediateContext->PSSetShader(mNanoSuitPixelShader, 0, 0);
	g_pImmediateContext->PSSetSamplers(0, 1, &mSamplerLinear);

	g_pImmediateContext->PSSetShaderResources(1, 1, &mStarCubeMap);

	mNanoSuitGameObject->Draw(mCamera.ViewProj());
	//draw sky

	mSky->Draw(g_pImmediateContext, mCamera);

	g_pSwapChain->Present(0, 0);
}

void LightingApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);

	mTotalTime += dt;
	mLightingConstantBuffer.data.capsuleLight.Len = (sinf(mTotalTime) * 2) * 5.0f;
	mLightingConstantBuffer.ApplyChanges();

	std::wostringstream s;
	s << mLightingConstantBuffer.data.capsuleLight.Len << std::endl;
	OutputDebugString(s.str().c_str());

	mLightRotationAngle += 0.2f * dt;
	XMMATRIX R = XMMatrixRotationY(mLightRotationAngle);
	mLightingConstantBuffer.data.gLightTransform = R;
}

void LightingApplication::BuildGeometryBuffer()
{
}


void LightingApplication::BuildConstantBuffer()
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
	mSky = new Sky(g_pd3dDevice,g_pImmediateContext, L"Textures/desertcube1024.dds", 10.0f);

	HR(CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/Star.dds", nullptr, &mStarCubeMap));

	HRESULT hr = cb_vs_vertexshader.Initialize(g_pd3dDevice, g_pImmediateContext);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The Constant Buffer cannot be Initialized.", L"Error", MB_OK);
		return;
	}

	hr = mLightingConstantBuffer.Initialize(g_pd3dDevice, g_pImmediateContext);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The Constant Buffer cannot be Initialized.", L"Error", MB_OK);
		return;
	}

	mNanoSuitGameObject = new GameObject();
	//mNanoSuitGameObject->Initialize("Models/Objects/nanosuit/nanosuit.obj", g_pd3dDevice, g_pImmediateContext, &cb_vs_vertexshader);
	mNanoSuitGameObject->Initialize("Models/Objects/nile/source/nile2.obj", g_pd3dDevice, g_pImmediateContext);
}

void LightingApplication::CleanupDevice()
{
	DirectX11Application::CleanupDevice();
	SafeDelete(mSky);
}

void LightingApplication::BuildFX()
{
	mSky->BuildSkyFX(g_pd3dDevice);
	BuildNanoSuitFX();
}

void LightingApplication::BuildNanoSuitFX()
{
	// Compile the vertex shader
	ID3DBlob* skyVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"FourCapsuleLight.hlsl", "VS", "vs_5_0", &skyVSBlob);
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
	hr = CompileShaderFromFile(L"FourCapsuleLight.hlsl", "PS", "ps_5_0", &skyPSBlob);
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
