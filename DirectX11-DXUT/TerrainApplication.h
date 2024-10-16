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
	Sky* mSky;
	Terrain mTerrain;

	DirectionalLight mDirLights[3];
	bool mIsWireFrame;
};
#endif


