#include "AnimationApplication.h"

#include <fstream>
#include <ratio>
#include "GameObject.h"
#include "GeometryGenerator.h"

AnimationApplication::AnimationApplication(HINSTANCE hinstance) : DirectX11Application(hinstance)
{
	mCamera.SetPosition(0.0f, 2.0f, -15.0f);
mCamera.CameraSpeed = 20.0f;


	mLightRotationAngle = 0.0f;

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

bool AnimationApplication::Init(int nShowCmd)
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

void AnimationApplication::DrawScene()
{
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	if (GetAsyncKeyState('1') & 1)
	{
		mIsWireFrame = !mIsWireFrame;
	}

	if (GetAsyncKeyState('2') & 1)
	{
		mBoneID -= 1.0f;
	}
	if (GetAsyncKeyState('3') & 1)
	{
		mBoneID+= 1.0f;
	}

	

	if (mIsWireFrame)
	{
		g_pImmediateContext->RSSetState(RenderStates::WireframeRS);
	}
	g_pImmediateContext->IASetInputLayout(InputLayouts::Animation);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_pImmediateContext->VSSetShader(mNanoSuitVertexShader, 0, 0);
	g_pImmediateContext->PSSetShader(mNanoSuitPixelShader, 0, 0);
	g_pImmediateContext->PSSetSamplers(0, 1, &mSamplerLinear);

	for (size_t i = 0; i < 3; i++)
	{
		mPerFrameBuffer.data.gDirLights[i] = mDirLights[i];
	}
	mPerFrameBuffer.data.gEyePosW = mCamera.GetPosition();
	mPerFrameBuffer.data.boneID = mBoneID;
	mPerFrameBuffer.ApplyChanges();
	mPerFrameBuffer.PSShaderUpdate(1);
	mPerFrameBuffer.VSShaderUpdate(1);
	
	mNanoSuitGameObject->Draw(mCamera.ViewProj());
	//draw sky

	mSky->Draw(g_pImmediateContext, mCamera);

	g_pSwapChain->Present(0, 0);
}

void AnimationApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);

	mLightRotationAngle += 0.002f * dt;

	//XMMATRIX R = XMMatrixRotationY(mLightRotationAngle);
	//XMVECTOR lightDir = XMVector3TransformNormal(XMLoadFloat3(&mDirLights[0].Direction), R);
	//XMStoreFloat3(&mDirLights[0].Direction, lightDir);
	mNanoSuitGameObject->UpdateModelAnimation(dt);
}

void AnimationApplication::BuildGeometryBuffer()
{
}


void AnimationApplication::BuildConstantBuffer()
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

	hr = mPerFrameBuffer.Initialize(g_pd3dDevice, g_pImmediateContext);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The Constant Buffer cannot be Initialized.", L"Error", MB_OK);
		return;
	}

	 hr = mSkinnedBuffer.Initialize(g_pd3dDevice, g_pImmediateContext);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The Constant Buffer cannot be Initialized.", L"Error", MB_OK);
		return;
	}

	HR(CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/grass.dds", nullptr, &mNanoSuitTexture));

	mNanoSuitGameObject = new GameObject();
	//mNanoSuitGameObject->Initialize("Models/Objects/nile/source/nile2.obj", g_pd3dDevice, g_pImmediateContext, &cb_vs_vertexshader);
	//mNanoSuitGameObject->Initialize("Models/Objects/TestModel/animModel.fbx", g_pd3dDevice, g_pImmediateContext, &cb_vs_vertexshader, &mSkinnedBuffer);
	mNanoSuitGameObject->Initialize("Models/Objects/boy.dae", g_pd3dDevice, g_pImmediateContext, &cb_vs_vertexshader, &mSkinnedBuffer);
	//mNanoSuitGameObject->Initialize("Models/Objects/SwingDance/SwingDance.fbx", g_pd3dDevice, g_pImmediateContext, &cb_vs_vertexshader, &mSkinnedBuffer);
	//mNanoSuitGameObject->Initialize("Models/Objects/chickenWarrior/source/ChickenWarrior2.fbx", g_pd3dDevice, g_pImmediateContext, &cb_vs_vertexshader, &mSkinnedBuffer);
	//mNanoSuitGameObject->Initialize("Models/Objects/SwingDance/Swing Dancing.fbx", g_pd3dDevice, g_pImmediateContext, &cb_vs_vertexshader, &mSkinnedBuffer);
	//mNanoSuitGameObject->Initialize("Models/Objects/frank/scene.gltf", g_pd3dDevice, g_pImmediateContext, &cb_vs_vertexshader);

}

void AnimationApplication::CleanupDevice()
{
	DirectX11Application::CleanupDevice();
	SafeDelete(mSky);
}

void AnimationApplication::BuildFX()
{
	mSky->BuildSkyFX(g_pd3dDevice);
	BuildNanoSuitFX();
}

void AnimationApplication::BuildNanoSuitFX()
{
	// Compile the vertex shader
	ID3DBlob* skyVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"BasicAnimation.fxh", "VS", "vs_5_0", &skyVSBlob);
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

	InputLayouts::BuildVertexLayout(g_pd3dDevice, skyVSBlob, InputLayoutDesc::Animation, ARRAYSIZE(InputLayoutDesc::Animation), &InputLayouts::Animation);
	if (FAILED(hr))
	{
		skyVSBlob->Release();
		return;
	}

	// Compile the pixel shader
	ID3DBlob* skyPSBlob = nullptr;
	hr = CompileShaderFromFile(L"BasicAnimation.fxh", "PS", "ps_5_0", &skyPSBlob);
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
