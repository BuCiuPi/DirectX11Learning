#ifndef BLENDINGAPPLICATION_H
#define BLENDINGAPPLICATION_H

#include "DirectX11Application.h"
#include "Waves.h"

class BlendingApplication : public DirectX11Application
{
public:
	BlendingApplication(HINSTANCE hInstance);
	virtual bool Init(int nShowCmd) override;

	virtual void DrawScene()override;
	virtual void UpdateScene(float dt)override;

	virtual void BuildGeometryBuffer() override;
	void BuildWaveBuffer();
	void BuildBoxBuffer();

	virtual void BuildConstantBuffer() override;


	virtual void CleanupDevice() override;

	float GetHeight(float x, float z) const;

	XMFLOAT3 GetHillNormal(float x, float z) const;

	virtual void BuildFX() override;

	void BuildBlendState();

private:
	ID3D11Buffer* mLandVB = nullptr;
	ID3D11Buffer* mLandIB = nullptr;

	ID3D11Buffer* mWavesVB = nullptr;
	ID3D11Buffer* mWavesIB = nullptr;

	ID3D11Buffer* mBoxVB = nullptr;
	ID3D11Buffer* mBoxIB = nullptr;

	ID3D11Buffer* mPerFrameBuffer = nullptr;
	ID3D11SamplerState* mSamplerLinear = nullptr;

	XMFLOAT4X4 mGrassTexTransform;
	XMFLOAT4X4 mWaterTexTransform;
	XMFLOAT4X4 mBoxTexTransform;
	XMFLOAT4X4 mLandWorld;
	XMFLOAT4X4 mWavesWorld;
	XMFLOAT4X4 mBoxWorld;

	ID3D11ShaderResourceView* mGrassMapSRV = nullptr;
	ID3D11ShaderResourceView* mWavesMapSRV = nullptr;
	ID3D11ShaderResourceView* mBoxMapSRV = nullptr;
	
	DirectionalLight mDirLights[3];

	Material mLandMat;
	Material mWavesMat;
	Material mBoxMat;

	UINT mLandIndexCount;
	UINT mBoxIndexCount;

	XMFLOAT2 mWaterTexOffset;

	ID3D11RasterizerState* mNoCullRS;
	ID3D11BlendState* mTransparentBlendState;

	Waves mWaves;

	UINT mGridIndexCount;
};

#endif // !BLENDINGAPPLICATION_H
