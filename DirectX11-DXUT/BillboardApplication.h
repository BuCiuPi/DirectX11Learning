#ifndef BILLBOARD_APPLICATION_H
#define BILLBOARD_APPLICATION_H
#include "DirectX11Application.h"
#include "Waves.h"
class BillboardApplication : public DirectX11Application
{
public:
	BillboardApplication(HINSTANCE hInstance);
	virtual bool Init(int nShowCmd) override;

	virtual void DrawScene()override;
	void DrawTreeSprite(WaveConstantBuffer cb);
	virtual void UpdateScene(float dt)override;

	virtual void BuildGeometryBuffer() override;
	void BuildWaveBuffer();
	void BuildBoxBuffer();
	void BuildTreeSpritesBuffer();


	virtual void BuildConstantBuffer() override;


	virtual void CleanupDevice() override;

	float GetHeight(float x, float z) const;

	XMFLOAT3 GetHillNormal(float x, float z) const;

	virtual void BuildFX() override;

	void BuildTreeFX();

	void BuildBlendState();

private:
	ID3D11Buffer* mLandVB = nullptr;
	ID3D11Buffer* mLandIB = nullptr;

	ID3D11Buffer* mWavesVB = nullptr;
	ID3D11Buffer* mWavesIB = nullptr;

	ID3D11Buffer* mBoxVB = nullptr;
	ID3D11Buffer* mBoxIB = nullptr;

	ID3D11Buffer* mTreeSpritesVB = nullptr;

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
	ID3D11ShaderResourceView* mTreeTextureMapArraySRV = nullptr;

	ID3D11Texture2D* mTreeTexture = nullptr;

	DirectionalLight mDirLights[3];

	Material mLandMat;
	Material mWavesMat;
	Material mBoxMat;
	Material mTreeMat;

	static const UINT TreeCount = 16;

	UINT mLandIndexCount;
	UINT mBoxIndexCount;

	XMFLOAT2 mWaterTexOffset;

	ID3D11RasterizerState* mNoCullRS;
	ID3D11BlendState* mTransparentBlendState;

	Waves mWaves;

	UINT mGridIndexCount;

	ID3D11VertexShader* mTreeVSShader;
	ID3D11PixelShader* mTreePSShader;
	ID3D11GeometryShader* mTreeGSShader;

	XMFLOAT4 mEyePosW;

};

#endif // !BILLBOARD_APPLICATION_H
