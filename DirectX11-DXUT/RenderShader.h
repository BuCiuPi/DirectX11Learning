#ifndef RENDER_SHADER_H
#define RENDER_SHADER_H
#include "D3DUtil.h"
class ShaderMaterial;

class RenderShader
{
public:
	virtual bool Initialize(ID3D11Device* device) = 0;
protected:
	RenderShader();
	~RenderShader();

	void LoadShader(ID3D11Device* device, const wchar_t* fileName, D3D11_INPUT_ELEMENT_DESC inputLayout[], int inputCount);
	void DrawRenderShader(ID3D11DeviceContext* deviceContext, int indexCount)const;

	ShaderMaterial* mShaderMaterial;
};
#endif
