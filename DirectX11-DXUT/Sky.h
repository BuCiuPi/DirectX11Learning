#ifndef SKY_H
#define SKY_H

#include <string>
#include "D3DUtil.h"

class Sky
{
public:
	Sky(ID3D11Device* device, const std::wstring& fileName, float skySphereRadius);
	~Sky();

	ID3D11ShaderResourceView* CubeMapSRV();

	bool BuildSkyFX(ID3D11Device* g_pd3dDevice);

	void Draw(ID3D11DeviceContext* dc, const Camera& camera);
private:
	//Sky(const Sky& rhs);
	//Sky& operator=(const Sky& rhs);

private:
	ID3D11VertexShader* mSkyVertexShader = nullptr;
	ID3D11PixelShader* mSkyPixelShader = nullptr;

	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	ID3D11Buffer* mSkyConstantBuffer;

	ID3D11ShaderResourceView* mCubeMapSRV;

	ID3D11SamplerState* mSamplerLinear;

	UINT mIndexCount;
};
#endif


