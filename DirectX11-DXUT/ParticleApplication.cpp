#include "ParticleApplication.h"

#include <fstream>

#include "GeometryGenerator.h"
#include "Terrain.h"

ParticleApplication::ParticleApplication(HINSTANCE hinstance) : DirectX11Application(hinstance)
{
	mCamera.SetPosition(0.0f, 50.0f, -15.0f);
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

bool ParticleApplication::Init(int nShowCmd)
{
	if (!D3DApp::Init(nShowCmd))
	{
		return false;
	}
	RenderStates::InitAll(g_pd3dDevice);

	BuildGeometryBuffer();
	BuildConstantBuffer();
	BuildFX();

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

	mTerrain.Init(g_pd3dDevice, g_pImmediateContext, tii);

	mFire.Init(g_pd3dDevice, mFlareTexSRV, mRandomTexSRV, 500);
	mFire.SetEmitPos(XMFLOAT3(0.0f, 50.0f, 0.0f));


	mRain.Init(g_pd3dDevice, mRainTexSRV, mRandomTexSRV, 10000);
	return true;
}

void ParticleApplication::DrawScene()
{
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	/// Draw Enviroment

	if (GetAsyncKeyState('1') & 1)
	{
		mIsWireFrame = !mIsWireFrame;
	}

	if (mIsWireFrame)
	{
		g_pImmediateContext->RSSetState(RenderStates::WireframeRS);
	}

	mTerrain.Draw(g_pImmediateContext, mCamera, mDirLights);

	//draw sky

	mSky->Draw(g_pImmediateContext, mCamera);

	/// Draw Particle
	mFire.SetEyePos(mCamera.GetPosition());
	mFire.SetBlendState(RenderStates::AdditiveBlendingBS);
	mRain.SetAccelerate(XMFLOAT3(0.0f, 2.0f, 0.0f));
	mFire.Draw(g_pImmediateContext, mCamera);

	mRain.SetEyePos(mCamera.GetPosition());
	mRain.SetEmitPos(mCamera.GetPosition());
	mRain.SetAccelerate(XMFLOAT3(-1.0f, -9.8f, 0.0f));
	mRain.Draw(g_pImmediateContext, mCamera);

	g_pSwapChain->Present(0, 0);
}

void ParticleApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);

	if (GetAsyncKeyState('R') & 0x8000)
	{
		mFire.Reset();
		mRain.Reset();
	}

	mFire.Update(dt, mTimer.GetTotalTime());
	mRain.Update(dt, mTimer.GetTotalTime());
}

void ParticleApplication::BuildGeometryBuffer()
{
}

void ParticleApplication::BuildConstantBuffer()
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

	//mSky = new Sky(g_pd3dDevice, L"Textures/grasscube1024.dds", 10.0f);
	//mSky = new Sky(g_pd3dDevice, L"Textures/snowcube1024.dds", 10.0f);
	mSky = new Sky(g_pd3dDevice,g_pImmediateContext, L"Textures/desertcube1024.dds", 10.0f);

	mRandomTexSRV = CreateRandomTexture1DSRV(g_pd3dDevice);
	
	LPCTSTR flaresTexName[] =
	{
		L"Textures\\flare0.dds",
	};
	ID3D11Texture2D* flareTexture = nullptr;
	HR(LoadTextureArray(g_pImmediateContext, g_pd3dDevice, flaresTexName, sizeof(flaresTexName) / sizeof(flaresTexName[0]), &flareTexture, &mFlareTexSRV));

	LPCTSTR rainTexName[] =
	{
		L"Textures\\raindrop.dds",
	};
	ID3D11Texture2D* rainTexture = nullptr;
	HR(LoadTextureArray(g_pImmediateContext, g_pd3dDevice, rainTexName, sizeof(rainTexName) / sizeof(rainTexName[0]), &rainTexture, &mRainTexSRV));

	ReleaseCOM(flareTexture);
	ReleaseCOM(rainTexture);
}

void ParticleApplication::CleanupDevice()
{
	DirectX11Application::CleanupDevice();
	SafeDelete(mSky);

	ReleaseCOM(mRandomTexSRV);
	ReleaseCOM(mRainTexSRV);
	ReleaseCOM(mFlareTexSRV);

	RenderStates::DestroyAll();
}

void ParticleApplication::BuildFX()
{
	mTerrain.BuildTerrainFX(g_pd3dDevice);
	mSky->BuildSkyFX(g_pd3dDevice);

	mFire.BuildFX(g_pd3dDevice, L"FireParticle.fxh", L"StreamOutParticle.fxh");
	mRain.BuildFX(g_pd3dDevice, L"RainParticle.fxh", L"RainStreamOutParticle.fxh");
}
