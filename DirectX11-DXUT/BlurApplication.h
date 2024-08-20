#ifndef BLURRAPPLICATION_H
#define BLURRAPPLICATION_H

#include "DirectX11Application.h"
#include "Waves.h"
#include "BlurFilter.h"

class BlurApplication : public DirectX11Application
{
public:
	BlurApplication(HINSTANCE hInstacne);

	virtual bool Init(int nShowCmd) override;

	virtual void DrawScene()override;
	void DrawSceneGeometry();
	void DrawSceneQuad();
	virtual void UpdateScene(float dt)override;

	virtual void BuildGeometryBuffer() override;
	void BuildLandGeometryBuffer();
	void BuildWaveBuffer();
	void BuildBoxBuffer();
	void BuidSceneQuadBuffer();

	virtual void OnResize() override;

	void BuildOffScreenView();

	virtual void BuildConstantBuffer() override;



	virtual void CleanupDevice() override;

	float GetHeight(float x, float z) const;

	XMFLOAT3 GetHillNormal(float x, float z) const;

	virtual void BuildFX() override;

	void BuildBlendingFX(bool& retFlag);
	void BuildComputeFX(bool& retFlag);
	void BuildQuadSceneFX(bool& retFlag);

	void BuildBlendState();

private:
	ID3D11Buffer* mLandVB = nullptr;
	ID3D11Buffer* mLandIB = nullptr;

	ID3D11Buffer* mWavesVB = nullptr;
	ID3D11Buffer* mWavesIB = nullptr;

	ID3D11Buffer* mBoxVB = nullptr;
	ID3D11Buffer* mBoxIB = nullptr;

	ID3D11Buffer* mScreenQuadVB = nullptr;
	ID3D11Buffer* mScreenQuadIB = nullptr;

	ID3D11Buffer* mPerFrameBuffer = nullptr;
	ID3D11Buffer* mBlurSettingBuffer = nullptr;
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

	ID3D11ShaderResourceView* mOffScreenSRV = nullptr;
	ID3D11UnorderedAccessView* mOffScreenUAV = nullptr;
	ID3D11RenderTargetView* mOffScreenRTV = nullptr;

	ID3D11InputLayout* mScreenQuadInputLayout = nullptr;
	ID3D11VertexShader* mScreenQuadVS = nullptr;
	ID3D11PixelShader* mScreenQuadPS = nullptr;

	BlurFilter mBlur;

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

#endif // !BLURRAPPLICATION_H
