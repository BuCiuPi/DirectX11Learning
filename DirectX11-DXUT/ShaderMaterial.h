#ifndef SHADER_MATERIAL_H
#define SHADER_MATERIAL_H

#include "D3DUtil.h"

enum ShaderMaterialType
{
	VertexShader,
	PixelShader
};

class ShaderMaterial
{
public:
	ShaderMaterial();
	~ShaderMaterial();

	ID3DBlob* BuildShader(ID3D11Device* device, const WCHAR* fileName, ShaderMaterialType type);
	ID3D11InputLayout* BuildInputLayout(ID3D11Device* device, ID3DBlob* blob,const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[], UINT numElements);
	void SetShader(ID3D11DeviceContext* deviceContext);
	void UnSetShader(ID3D11DeviceContext* deviceContext);

private:
	ID3DBlob* BuildVertexShader(ID3D11Device* device, const WCHAR* fileName);
	ID3DBlob* BuildPixelShader(ID3D11Device* device, const WCHAR* fileName);
	ID3D11PixelShader* mPixelShader;
	ID3D11InputLayout* mInputLayout;
	ID3D11VertexShader* mVertexShader;
};
#endif


