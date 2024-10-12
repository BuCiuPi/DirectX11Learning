#ifndef SKY_BOX_SHADER_H
#define SKY_BOX_SHADER_H
#include "RenderShader.h"

class SkyBoxShader : public RenderShader
{
public:;
	SkyBoxShader();
	virtual ~SkyBoxShader();

	bool Initialize(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, ConstantBuffer<FrameBufferType>* frameBuffer);
private:
	ID3D11SamplerState* mSampler;
};
#endif
