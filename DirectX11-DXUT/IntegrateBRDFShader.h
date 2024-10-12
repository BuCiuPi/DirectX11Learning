#pragma once
#include "RenderShader.h"

class IntegrateBRDFShader : public RenderShader
{
public:
	IntegrateBRDFShader();
	virtual ~IntegrateBRDFShader();

	bool Initialize(ID3D11Device* device) override;
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount,ConstantBuffer<FrameBufferType>* frameBuffer) const;

private:
	ID3D11SamplerState* mSampler;

};

