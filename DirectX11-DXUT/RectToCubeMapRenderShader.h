#ifndef RECT_TO_CUBE_MAP_RENDER_SHADER_H
#define RECT_TO_CUBE_MAP_RENDER_SHADER_H
#include "RenderShader.h"

#include "D3DUtil.h"

class RectToCubeMapRenderShader : public RenderShader
{
	
public :
	RectToCubeMapRenderShader();
	virtual ~RectToCubeMapRenderShader();

	bool Initialize(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, ConstantBuffer<FrameBufferType>* frameBuffer);
private:
	ID3D11SamplerState* mSampler;
};
#endif


