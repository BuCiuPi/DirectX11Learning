#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "D3DUtil.h"

class ShadowMap
{
public:
	ShadowMap(ID3D11Device* device, UINT width, UINT height);
	~ShadowMap();

	ID3D11ShaderResourceView* DepthMapSRV();

	void BindDsvAndSetNullRenderTarget(ID3D11DeviceContext* dc);
	void UnBindDSV(ID3D11DeviceContext* dc);

private:

	UINT mWidth;	
	UINT mHeight;	

	ID3D11ShaderResourceView* mDepthMapSRV;
	ID3D11DepthStencilView* mDepthMapDSV;

	D3D11_VIEWPORT mViewPort;

};

#endif
