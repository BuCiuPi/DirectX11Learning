#ifndef TESSELLATION_APPLICATION_H
#define TESSELLATION_APPLICATION_H

#include "DirectX11Application.h"
class TessellationApplication : public DirectX11Application
{
public:
	TessellationApplication(HINSTANCE hInstance);

	virtual bool Init(int nShowCmd) override;

	virtual void DrawScene()override;
	virtual void UpdateScene(float dt)override;

	virtual void BuildGeometryBuffer() override;

	virtual void BuildConstantBuffer() override;


	virtual void CleanupDevice() override;

	virtual void BuildFX() override;

private:

	ID3D11HullShader* mHullShader = nullptr;
	ID3D11DomainShader* mDomainShader = nullptr;

	ID3D11Buffer* mLandVB = nullptr;

	ID3D11Buffer* mPerFrameBuffer = nullptr;
	ID3D11SamplerState* mSamplerLinear = nullptr;

	XMFLOAT4X4 mGrassTexTransform;
	XMFLOAT4X4 mLandWorld;

	ID3D11ShaderResourceView* mGrassMapSRV = nullptr;

	DirectionalLight mDirLights[3];

	Material mLandMat;
};

#endif


