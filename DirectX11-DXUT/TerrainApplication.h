#ifndef TERRAIN_APPLICATION_H
#define TERRAIN_APPLICATION_H

#include "DirectX11Application.h"
#include "Terrain.h"

class TerrainApplication : public DirectX11Application
{
public:
	TerrainApplication(HINSTANCE hInstance);

	virtual bool Init(int nShowCmd) override;

	virtual void DrawScene()override;
	virtual void UpdateScene(float dt)override;

	virtual void BuildGeometryBuffer() override;

	virtual void BuildConstantBuffer() override;

	virtual void CleanupDevice() override;

	virtual void BuildFX() override;

private:

	ID3D11VertexShader* mSkyVertexShader = nullptr;
	ID3D11PixelShader* mSkyPixelShader = nullptr;


	ID3D11Buffer* mTerrainConstantBuffer = nullptr;
	ID3D11Buffer* mTerrainPerFrameBuffer = nullptr;

	ID3D11VertexShader* mTerrainVertexShader = nullptr;
	ID3D11HullShader* mTerrainHullShader = nullptr;
	ID3D11DomainShader* mTerrainDomainShader = nullptr;
	ID3D11PixelShader* mTerrainPixelShader = nullptr;

	ID3D11Buffer* mSkyConstantBuffer = nullptr;

	Sky* mSky;

	ID3D11Buffer* mSkySphereVB = nullptr;
	ID3D11Buffer* mSkySphereIB = nullptr;

	ID3D11Buffer* mPerFrameBuffer = nullptr;
	ID3D11SamplerState* mSamplerLinear = nullptr;

	XMFLOAT4X4 mMeshTexTransform;
	XMFLOAT4X4 mMeshWorld;

	ID3D11ShaderResourceView* nullSRV = nullptr;

	DirectionalLight mDirLights[3];

	Terrain mTerrain;

	bool mFrustumCullingEnabled = true;
};
#endif


