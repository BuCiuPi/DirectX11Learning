#ifndef PRE_FILTER_SHADER_H
#define PRE_FILTER_SHADER_H
#include "D3DUtil.h"
#include "RenderShader.h"
#include "Sky.h"

class PreFilterShader : public RenderShader
{
public:
	PreFilterShader();
	virtual ~PreFilterShader();

	bool Initialize(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, ConstantBuffer<FrameBufferType>* frameBuffer) const;

private:
	ID3D11SamplerState* mSampler;

};
#endif


