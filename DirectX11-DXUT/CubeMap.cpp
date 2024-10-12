#include "CubeMap.h"

CubeMap::CubeMap()
	: mShaderResourceView(NULL), mTexture(NULL), mMipMaps(0)
{
}

CubeMap::~CubeMap()
{
	SafeDelete(mShaderResourceView);
	SafeDelete(mTexture);
}

bool CubeMap::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, std::vector<RenderTexture*> faces,
	int width, int height, int mipMaps)
{
	mMipMaps = mipMaps;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = mipMaps;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &mTexture);
	if (FAILED(hr))
	{
		return  false;
	}

	if (!faces.empty())
	{
		Copy(context, faces, width, height, 0);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(mTexture, &srvDesc, &mShaderResourceView);
	if (FAILED(hr))
	{
		return  false;
	}
	return true;
}

void CubeMap::Copy(ID3D11DeviceContext* context, std::vector<RenderTexture*> faces, int width, int height,
	int mipSlice) const
{
	D3D11_BOX sourceRegion;
	for (int i = 0; i < 6; ++i)
	{
		RenderTexture* texture = faces[i];

		sourceRegion.left = 0;
		sourceRegion.right = width;
		sourceRegion.top = 0;
		sourceRegion.bottom = height;
		sourceRegion.front = 0;
		sourceRegion.back = 1;

		context->CopySubresourceRegion(mTexture, D3D11CalcSubresource(mipSlice, i, mMipMaps), 0, 0, 0, texture->GetTexture(),
			0, &sourceRegion);
	}
}

ID3D11Texture2D* CubeMap::GetTexture() const
{
	return mTexture;
}

ID3D11ShaderResourceView* CubeMap::GetSRV() const
{
	return mShaderResourceView;
}
