#ifndef WAVE_APPLICATION_H
#define WAVE_APPLICATION_H

#include "DirectX11Application.h"
#include "Waves.h"

class WaveApplication :public DirectX11Application
{
public:
	WaveApplication(HINSTANCE hInstance);

	virtual bool Init(int nShowCmd) override;

	virtual void DrawScene()override;
	virtual void UpdateScene(float dt)override;

	virtual void BuildGeometryBuffer() override;
	void BuildWaveBuffer();

	virtual void BuildConstantBuffer() override;

	virtual HRESULT BuildVertexLayout(ID3DBlob* pVSBlob) override;

	virtual void CleanupDevice() override;

	float GetHeight(float x, float z) const;
	XMFLOAT3 GetHillNormal(float x, float z) const;

	virtual void BuildFX() override;

private:
	ID3D11Buffer* mLandVB = nullptr;
	ID3D11Buffer* mLandIB = nullptr;

	ID3D11Buffer* mWavesVB = nullptr;
	ID3D11Buffer* mWavesIB = nullptr;

	ID3D11Buffer* mPerFrameBuffer = nullptr;
	ID3D11SamplerState* mSamplerLinear = nullptr;

	XMFLOAT4X4 mGrassTexTransform;
	XMFLOAT4X4 mWaterTexTransform;
	XMFLOAT4X4 mLandWorld;
	XMFLOAT4X4 mWavesWorld;

	ID3D11ShaderResourceView* mGrassMapSRV = nullptr;
	ID3D11ShaderResourceView* mWavesMapSRV = nullptr;

	DirectionalLight mDirLights[3];

	Material mLandMat;
	Material mWavesMat;

	UINT mLandIndexCount;

	XMFLOAT2 mWaterTexOffset;



	Waves mWaves;

	UINT mGridIndexCount;
};
#endif // !WAVE_APPLICATION_H
