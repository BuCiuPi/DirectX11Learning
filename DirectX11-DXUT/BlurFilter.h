#ifndef BLURFILTER_H
#define BLURFILTER_H

#include "D3DUtil.h"

struct ConstantBlurBuffer
{
	float gWeights[12] =
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f, 0.05f
	};
};

class BlurFilter
{
public:
	BlurFilter();
	~BlurFilter();
	ID3D11ShaderResourceView* GetBlurOutput();

	void Init(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format );

	void BlurInPlace(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* inputSRV, ID3D11UnorderedAccessView* inputUAV, int blurCount);

	ID3D11ComputeShader* mHorzBlurCS = nullptr;
	ID3D11ComputeShader* mVertBlurCS = nullptr;
private:

	UINT mWidth;
	UINT mHeight;
	DXGI_FORMAT mFormat;

	ID3D11ShaderResourceView* mBLurredOutputTexSRV = nullptr;
	ID3D11UnorderedAccessView* mBLurredOutputTexUAV = nullptr;

	ID3D11Buffer* mConstantBlurBuffer = nullptr;

};

#endif // !BLURFILTER_H
