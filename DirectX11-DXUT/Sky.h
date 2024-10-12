#ifndef SKY_H
#define SKY_H

#include "D3DUtil.h"
#include "D3DApp.h"
#include "RenderTexture.h"

class SkyBoxShader;

struct SkyBoxConstantBuffer
{
	XMMATRIX mMVP;
};

struct FrameBufferType
{
	XMMATRIX View;
	XMMATRIX Projection;
	XMFLOAT4 LightPositions[4];
	XMFLOAT4 LightColours[4];
	XMFLOAT3 CamPos;
	float pad;
	XMFLOAT4 CustomData;
};

struct ObjectBufferType
{
	XMMATRIX World;
};

struct PosUvVertexType
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 Uv;
};

class CubeMap;
class ShaderMaterial;
class D3DApp;
class Sky
{
public:
	Sky(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& fileName, float skySphereRadius);
	~Sky();

	ID3D11ShaderResourceView* CubeMapSRV();

	bool BuildSkyFX(ID3D11Device* g_pd3dDevice);

	void Draw(ID3D11DeviceContext* dc, const Camera& camera);
	void Render(ID3D11DeviceContext* dc, const Camera& camera);
	void CreateCubeBuffer(ID3D11Device* device);

	bool CreateCubeMap(D3DApp* app, const wchar_t* fileName);
	void BindMesh(ID3D11DeviceContext* deviceContext) const;
	//private:
	//Sky(const Sky& rhs);
	//Sky& operator=(const Sky& rhs);

private:
	ID3D11InputLayout* mInputLayout;

	ShaderMaterial* mSkyShaderMaterial;
	SkyBoxShader* mSkyBoxShader;

	CubeMap* mCubeMap;
	CubeMap* mIrradianceMap;
	CubeMap* mPreFilterMap;

	RenderTexture* mBrdfLUT;

	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	ID3D11Buffer* mCVB;
	ID3D11Buffer* mCIB;

	ID3D11ShaderResourceView* mCubeMapSRV;
	ID3D11SamplerState* mSamplerLinear;
	ConstantBuffer<SkyBoxConstantBuffer> mSkyConstantBuffer;
	ConstantBuffer<FrameBufferType> mSkyCubeConstantBuffer;

	const int mSkyBoxSize = 2048;
	const int mIrradianceSize = 32;
	const int PreFilterSize = 256;
	const int BrdfLookupSize = 512;

	UINT mIndexCount;
	UINT mVertexCount;

	UINT mCubeIndexCount;
	UINT mCubeVertexCount;
	Camera* mCubeCamera;
};
#endif


