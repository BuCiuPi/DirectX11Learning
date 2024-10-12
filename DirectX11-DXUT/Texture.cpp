#include "Texture.h"

#include <istream>

Texture::Texture()
	:mShaderResourceView(NULL), mTexture(NULL)
{
}

Texture::~Texture()
{
	SAFE_RELEASE(mShaderResourceView);
	SAFE_RELEASE(mTexture);
}

bool Texture::Initialize(ID3D11Device* device, const wchar_t* fileName)
{
	ID3D11Resource* texture;
	const HRESULT result = CreateDDSTextureFromFile(device, fileName, &texture, &mShaderResourceView);
	if (FAILED(result))
	{
		return false;
	}

	mTexture = static_cast<ID3D11Texture2D*>(texture);
}

ID3D11Texture2D* Texture::GetTexture() const
{
	return mTexture;
}

ID3D11ShaderResourceView* Texture::GetSRV() const
{
	return mShaderResourceView;
}