#ifndef TEXTURE_H
#define TEXTURE_H

#include "D3DUtil.h"

class Texture
{
public:
	Texture();
	~Texture();

	bool Initialize(ID3D11Device* device, const wchar_t* fileName);

	ID3D11Texture2D* GetTexture() const;
	ID3D11ShaderResourceView* GetSRV() const;
	
private:
	ID3D11Texture2D* mTexture;
	ID3D11ShaderResourceView* mShaderResourceView;

};
#endif


