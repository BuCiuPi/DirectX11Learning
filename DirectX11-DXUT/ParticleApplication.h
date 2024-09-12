#ifndef PARTICLE_APPLICATION_H
#define PARTICLE_APPLICATION_H

#include "DirectX11Application.h"
#include "ParticleSystem.h"
#include "Terrain.h"

class ParticleApplication : public DirectX11Application
{
public:
	ParticleApplication(HINSTANCE hInstance);
	
	virtual bool Init(int nShowCmd) override;

	virtual void DrawScene()override;
	virtual void UpdateScene(float dt)override;

	virtual void BuildGeometryBuffer() override;

	virtual void BuildConstantBuffer() override;

	virtual void CleanupDevice() override;

	virtual void BuildFX() override;

private:
	
	ID3D11ShaderResourceView* mFlareTexSRV = nullptr;
	ID3D11ShaderResourceView* mRainTexSRV = nullptr;
	ID3D11ShaderResourceView* mRandomTexSRV = nullptr;

	ParticleSystem mFire;
	ParticleSystem mRain;

	Sky* mSky;
	Terrain mTerrain;

	DirectionalLight mDirLights[3];
	bool mIsWireFrame = false;
};
#endif


