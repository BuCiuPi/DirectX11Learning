#ifndef CUBE_MAP_H
#define CUBE_MAP_H
#include <vector>
#include "D3DUtil.h"

class CubeMap
{
public:
	CubeMap();
	~CubeMap();

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, std::vector<RenderTexture*> faces, int width,
		int height, int mipMaps);

	void Copy(ID3D11DeviceContext* context, std::vector<RenderTexture*> faces, int width, int height, int mipSlice) const;

	ID3D11Texture2D* GetTexture() const;
	ID3D11ShaderResourceView* GetSRV() const;
private:
	ID3D11Texture2D* mTexture;
	ID3D11ShaderResourceView* mShaderResourceView;
	int mMipMaps;
};
#endif

