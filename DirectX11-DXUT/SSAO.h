#ifndef SSAO_H
#define SSAO_H
#include "D3DUtil.h"

struct NormalDepthConstantBuffer
{
	XMMATRIX gViewToTexSpace;
	XMFLOAT4 gOffsetVectors[14];
	XMFLOAT4 gFrustumCorners[4];

	float gOcclusionRadius = 0.5f;
	float gOcclusionFadeStart = 0.2f;
	float gOcclusionFadeEnd = 2.0f;
	float gSurfaceEpsilon = 0.05f;
};

class SSAO
{
public:
	SSAO(ID3D11Device* device, ID3D11DeviceContext* dc, int width, int height, float fovy, float farZ);
	~SSAO();

	ID3D11ShaderResourceView* NormalDepthSRV();
	ID3D11ShaderResourceView* AmbientSRV();

	void OnSize(int width, int height, float fovy, float farZ);

	void SetNormalDepthRenderTarget(ID3D11DepthStencilView* dsv);

	void ComputeSSAO(const Camera& camera);

	void BlurAmientMap(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV, bool horzBlur);
	void BlurAmientMap(int blurCount);

	void BuildRandomVectorTexture();
private:

	void BuildFrushtumFarCorners(float fovy, float farZ);
	void BuildFullScreenQuad();

	void BuildTextureViews();
	void ReleaseTextureViews();

	void BuildOffsetVectors();

	void DrawFullScreenQuad();
	
	void BuildNormalDepthFX();
	void BuildNormalDepthConstantBuffer();
private:

	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* mDC;

	ID3D11Buffer* mScreenQuadVB;
	ID3D11Buffer* mScreenQuadIB;

	ID3D11ShaderResourceView* mRandomVectorSRV;
	ID3D11SamplerState* mSamRandomVec;

	ID3D11Buffer* mNormalDepthConstantBuffer;
	ID3D11SamplerState* mSamNormalDepth;

	ID3D11VertexShader* mNormalDepthVertexShader;
	ID3D11PixelShader* mNormalDepthPixelShader;

	NormalDepthConstantBuffer ndcb;



	ID3D11RenderTargetView* mNormalDepthRTV;
	ID3D11ShaderResourceView* mNormalDepthSRV;

	ID3D11RenderTargetView* mAmbientRTV0;
	ID3D11ShaderResourceView* mAmbientSRV0;

	ID3D11RenderTargetView* mAmbientRTV1;
	ID3D11ShaderResourceView* mAmbientSRV1;

	UINT mRenderTargetWidth;
	UINT mRenderTargetHeight;


	D3D11_VIEWPORT mAmbientMapViewport;

};


#endif


