#include "DeferredShadingApplication.h"

#include <fstream>
#include <ratio>
#include <sstream>

#include "GameObject.h"
#include "GeometryGenerator.h"

DeferredShadingApplication::DeferredShadingApplication(HINSTANCE hinstance) : DirectX11Application(hinstance)
{
	mCamera.SetPosition(0.0f, 10.0f, -15.0f);
	mCamera.CameraSpeed = 50.0f;

	this->mMaterial.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	this->mMaterial.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	this->mMaterial.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
	this->mMaterial.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	XMFLOAT3 ambientLowerColor = XMFLOAT3(0.1f, 0.2f, 0.1f);
	XMFLOAT3 ambientUpperColor = XMFLOAT3(0.1f, 0.2f, 0.2f);

	XMVECTOR directionalLightDir = XMVectorSet(1.0f, -1.0f, 0.0f, 1.0f);
	XMFLOAT3 directionalLightColor = XMFLOAT3(0.85f, 0.8f, 0.5f);

	mLightManager.SetAmbient(ambientLowerColor, ambientUpperColor);
	mLightManager.SetDirectional(directionalLightDir, directionalLightColor);
}

bool DeferredShadingApplication::Init(int nShowCmd)
{
	if (!D3DApp::Init(nShowCmd))
	{
		return false;
	}
	RenderStates::InitAll(g_pd3dDevice);

	InitScene();

	BuildGeometryBuffer();
	BuildConstantBuffer();

	BuildFX();

	return true;
}

void DeferredShadingApplication::DrawScene()
{
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	if (GetAsyncKeyState('1') & 1)
	{
		mIsWireFrame = !mIsWireFrame;
	}

	//if (mIsWireFrame)
	//{
	//	g_pImmediateContext->RSSetState(RenderStates::WireframeRS);
	//}

	g_pImmediateContext->PSSetSamplers(0, 1, &mSamplerLinear);

	mCgBuffer.PreRender(g_pImmediateContext);
	mSceneManager.RenderScene(g_pImmediateContext, &mCamera);
	mCgBuffer.PostRender(g_pImmediateContext);

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, mCgBuffer.GetDepthReadOnlyDSV());
	mCgBuffer.PrepareForUnpack(g_pImmediateContext, &mCamera);

	mLightManager.DoLighting(g_pImmediateContext, &mCgBuffer);

	//g_pImmediateContext->IASetInputLayout(mInputLayout);
	//g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//g_pImmediateContext->PSSetSamplers(0, 1, &mSamplerLinear);

	//// set constant buffer
	//XMMATRIX worldMatrix = mNanoSuitGameObject->GetWorldMatrix();
	//XMMATRIX MVP = worldMatrix * mCamera.ViewProj();
	//this->cb_vs_vertexshader.data.gWorld = XMMatrixTranspose(worldMatrix);
	//this->cb_vs_vertexshader.data.gWorldViewProj = XMMatrixTranspose(MVP);
	//this->cb_vs_vertexshader.data.material = mMaterial;
	//this->cb_vs_vertexshader.ApplyChanges();
	//this->cb_vs_vertexshader.VSShaderUpdate(0);

	//// Draw
	//mNanoSuitGameObject->Draw(mCamera.ViewProj());
	//draw sky

	g_pSwapChain->Present(0, 0);
}

void DeferredShadingApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);
}

void DeferredShadingApplication::BuildGeometryBuffer()
{
}


void DeferredShadingApplication::BuildConstantBuffer()
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

	HR(CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/Star.dds", nullptr, &mStarCubeMap));

}

void DeferredShadingApplication::CleanupDevice()
{
	DirectX11Application::CleanupDevice();
}

void DeferredShadingApplication::BuildFX()
{
	BuildNanoSuitFX();
}

void DeferredShadingApplication::BuildNanoSuitFX()
{
	ID3DBlob* vsBlob = mShaderMaterial->BuildShader(g_pd3dDevice, L"BasicUnlit.hlsl", VertexShader);
	mInputLayout = mShaderMaterial->BuildInputLayout(g_pd3dDevice, vsBlob, InputLayoutDesc::Basic32, ARRAYSIZE(InputLayoutDesc::Basic32));
	mShaderMaterial->BuildShader(g_pd3dDevice, L"BasicUnlit.hlsl", PixelShader);
}

void DeferredShadingApplication::InitScene()
{
	HRESULT hr = cb_vs_vertexshader.Initialize(g_pd3dDevice, g_pImmediateContext);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The Constant Buffer cannot be Initialized.", L"Error", MB_OK);
		return;
	}

	mShaderMaterial = new ShaderMaterial();
	mNanoSuitGameObject = new GameObject();
	mNanoSuitGameObject->Initialize("Models/Objects/nile/source/nile2.obj", g_pd3dDevice, g_pImmediateContext);
	//mNanoSuitGameObject->Initialize("Models/Objects/nanosuit/nanosuit.obj", g_pd3dDevice, g_pImmediateContext);


	mCgBuffer.Init(g_pd3dDevice, mClientWidth, mClientHeight);
	mSceneManager.Init(g_pd3dDevice, g_pImmediateContext);
	mLightManager.Init(g_pd3dDevice, g_pImmediateContext);


	std::vector<GameObject*> gameObjects;

	GameObject* newGameObject = new GameObject(mNanoSuitGameObject);	
	gameObjects.push_back(newGameObject);

	newGameObject = new GameObject(mNanoSuitGameObject);	
	newGameObject->SetPosition(-50.0f, 0.0f, 0.0f);
	newGameObject->SetRotation(0.0f, 90.0f, 0.0f);
	gameObjects.push_back(newGameObject);
	
	newGameObject = new GameObject(mNanoSuitGameObject);
	newGameObject->SetPosition(50.0f, 0.0f, 0.0f);
	newGameObject->SetRotation(0.0f, -90.0f, 0.0f);
	gameObjects.push_back(newGameObject);

	mSceneManager.SetVisibleGameObject(gameObjects);
}
