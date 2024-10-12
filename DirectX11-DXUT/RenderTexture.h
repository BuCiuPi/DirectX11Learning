#ifndef RENDER_TEXTURE_H
#define RENDER_TEXTURE_H
#include <d3d11.h>
class D3DApp;
class RenderTexture
{
public:
	RenderTexture();
	~RenderTexture();

	bool Initialize(ID3D11Device* device, int width, int height, int mipMaps);

	void SetRenderTarget(D3DApp* app, ID3D11DeviceContext* deviceContext) const;
	void ClearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, float red, float green, float blue, float alpha);

	ID3D11ShaderResourceView* GetSRV() const;
	ID3D11Texture2D* GetTexture() const;

private:
	int mWidth;
	int mHeight;

	ID3D11ShaderResourceView* mShaderResourceView;
	ID3D11Texture2D* mRenderTargetTexture;
	ID3D11RenderTargetView* mRenderTargetView;
};
#endif


