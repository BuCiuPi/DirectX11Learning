#include "RenderShader.h"

#include "ShaderMaterial.h"

RenderShader::RenderShader()
{
}

RenderShader::~RenderShader()
{
	SAFE_DELETE(mShaderMaterial)
}

void RenderShader::LoadShader(ID3D11Device* device, const wchar_t* fileName, D3D11_INPUT_ELEMENT_DESC inputLayout[],
	int inputCount)
{
	mShaderMaterial = new ShaderMaterial();
	ID3DBlob* blob = mShaderMaterial->BuildShader(device, fileName, VertexShader);
	mShaderMaterial->BuildInputLayout(device, blob, inputLayout, inputCount);
	mShaderMaterial->BuildShader(device, fileName, PixelShader);
}

void RenderShader::DrawRenderShader(ID3D11DeviceContext* deviceContext, int indexCount) const
{
	mShaderMaterial->SetInputLayout(deviceContext);
	mShaderMaterial->SetShader(deviceContext);

	deviceContext->DrawIndexed(indexCount, 0, 0);
}
