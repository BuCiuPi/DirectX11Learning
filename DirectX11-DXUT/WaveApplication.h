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

	float GetHeight(float x, float z) const;

private:
	ID3D11Buffer* mLandVB;
	ID3D11Buffer* mLandIB;
	ID3D11Buffer* mWavesVB;
	ID3D11Buffer* mWavesIB;

	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mWavesWorld;

	Waves mWaves;

	UINT mGridIndexCount;
};
#endif // !WAVE_APPLICATION_H
