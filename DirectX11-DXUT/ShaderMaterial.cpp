#include "ShaderMaterial.h"

ShaderMaterial::ShaderMaterial()
	: mPixelShader(NULL), mVertexShader(NULL), mInputLayout(NULL)
{
}

ShaderMaterial::~ShaderMaterial()
{
	SAFE_RELEASE(mPixelShader);
	SAFE_RELEASE(mVertexShader);
}

ID3DBlob* ShaderMaterial::BuildShader(ID3D11Device* device, const WCHAR* fileName, ShaderMaterialType type)
{
	switch (type)
	{
	case VertexShader:
		return BuildVertexShader(device, fileName);
		break;

	case PixelShader:
		return BuildPixelShader(device, fileName);
		break;

	default:
		return nullptr;
	}
}

ID3D11InputLayout* ShaderMaterial::BuildInputLayout(ID3D11Device* device, ID3DBlob* blob, const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[], UINT numElements)
{
	InputLayouts::BuildVertexLayout(device, blob, inputLayoutDesc, numElements, &mInputLayout);
	return mInputLayout;
}

void ShaderMaterial::SetShader(ID3D11DeviceContext* deviceContext)
{
	deviceContext->VSSetShader(mVertexShader, 0, 0);
	deviceContext->PSSetShader(mPixelShader, 0, 0);
}

void ShaderMaterial::UnSetShader(ID3D11DeviceContext* deviceContext)
{
	deviceContext->VSSetShader(NULL, NULL, 0);
	deviceContext->PSSetShader(NULL, NULL, 0);
}

ID3DBlob* ShaderMaterial::BuildVertexShader(ID3D11Device* device, const WCHAR* fileName)
{
	ID3DBlob* vsBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(fileName, "VS", "vs_5_0", &vsBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return nullptr;
	}

	// Create the vertex shader
	hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &mVertexShader);
	if (FAILED(hr))
	{
		vsBlob->Release();
		return nullptr;
	}

	return vsBlob;
}

ID3DBlob* ShaderMaterial::BuildPixelShader(ID3D11Device* device, const WCHAR* fileName)
{
	// Compile the pixel shader
	ID3DBlob* psBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(fileName, "PS", "ps_5_0", &psBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return nullptr;
	}

	// Create the pixel shader
	hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mPixelShader);
	psBlob->Release();
	if (FAILED(hr))
		return nullptr;

	return psBlob;
}
