#ifndef IRRADIANCE_SHADER_H
#define IRRADIANCE_SHADER_H
#include "RenderShader.h"

class IrradianceShader : public RenderShader
{
public:
	IrradianceShader();
	virtual ~IrradianceShader();

	bool Initialize(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, ConstantBuffer<FrameBufferType>* frameBuffer) const;

private:
	ID3D11SamplerState* mSampler;
};
#endif

