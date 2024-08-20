#include "BlurFilter.h"

BlurFilter::BlurFilter()
{
}

BlurFilter::~BlurFilter()
{
	ReleaseCOM(mBLurredOutputTexSRV);
	ReleaseCOM(mBLurredOutputTexUAV);
}

ID3D11ShaderResourceView* BlurFilter::GetBlurOutput()
{
	return mBLurredOutputTexSRV;
}

void BlurFilter::Init(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format)
{
	ReleaseCOM(mBLurredOutputTexSRV);
	ReleaseCOM(mBLurredOutputTexUAV);

	mWidth = width;
	mHeight = height;
	mFormat = format;

	D3D11_TEXTURE2D_DESC blurTexDesc;
	blurTexDesc.Width = mWidth;
	blurTexDesc.Height = mHeight;
	blurTexDesc.MipLevels = 1;
	blurTexDesc.ArraySize = 1;
	blurTexDesc.Format = mFormat;
	blurTexDesc.SampleDesc.Count = 1;
	blurTexDesc.SampleDesc.Quality = 0;
	blurTexDesc.Usage = D3D11_USAGE_DEFAULT;
	blurTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	blurTexDesc.CPUAccessFlags = 0;
	blurTexDesc.MiscFlags = 0;

	ID3D11Texture2D* blurredTex = 0;
	HR(device->CreateTexture2D(&blurTexDesc, 0, &blurredTex));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = mFormat;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	HR(device->CreateShaderResourceView(blurredTex, &srvDesc, &mBLurredOutputTexSRV));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = mFormat;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	HR(device->CreateUnorderedAccessView(blurredTex, &uavDesc, &mBLurredOutputTexUAV));

	ReleaseCOM(blurredTex);

	D3D11_BUFFER_DESC bd;
	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBlurBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	HR(device->CreateBuffer(&bd, nullptr, &mConstantBlurBuffer));
}

void BlurFilter::BlurInPlace(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* inputSRV, ID3D11UnorderedAccessView* inputUAV, int blurCount)
{

	ConstantBlurBuffer blurBuffer;
	deviceContext->UpdateSubresource(mConstantBlurBuffer, 0, nullptr, &blurBuffer, 0, 0);
	deviceContext->CSSetConstantBuffers(0, 1, &mConstantBlurBuffer);

	for (int i = 0; i < blurCount; i++)
	{
		deviceContext->CSSetShader(mHorzBlurCS, nullptr, 0);
		// Horizontal Blurr
		deviceContext->CSSetShaderResources(0, 1, &inputSRV);
		deviceContext->CSSetUnorderedAccessViews(0, 1, &mBLurredOutputTexUAV, 0);

		UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
		deviceContext->Dispatch(numGroupsX, mHeight, 1);

		// unbind
		ID3D11ShaderResourceView* nullSRV[1] = { 0 };
		deviceContext->CSSetShaderResources(0, 1, nullSRV);
		ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
		deviceContext->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

		deviceContext->CSSetShader(mVertBlurCS, nullptr, 0);

		deviceContext->CSSetShaderResources(0, 1, &mBLurredOutputTexSRV);
		deviceContext->CSSetUnorderedAccessViews(0, 1, &inputUAV, 0);

		UINT numGroupsY = (UINT)ceilf(mHeight / 256.0f);
		deviceContext->Dispatch(mWidth, numGroupsY, 1);

		deviceContext->CSSetShaderResources(0, 1, nullSRV);
		deviceContext->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	}

	deviceContext->CSSetShader(0, 0, 0);
}