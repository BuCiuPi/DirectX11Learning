#include "ModelTexture.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include "StringHelper.h"

ModelTexture::ModelTexture(ID3D11Device* device, const ColorHelper& color, aiTextureType type)
{
	this->Initialize1x1ColorTexture(device, color, type);
}

ModelTexture::ModelTexture(ID3D11Device* device, const ColorHelper* colorData, UINT width, UINT height, aiTextureType type)
{
	this->InitializeColorTexture(device, colorData, width, height, type);
}

ModelTexture::ModelTexture(ID3D11Device* device, const std::string& filePath, aiTextureType type)
{
	this->type = type;
	if (StringHelper::GetFileExtension(filePath) == ".dds")
	{
		HRESULT hr = DirectX::CreateDDSTextureFromFile(device, StringHelper::StringToWide(filePath).c_str(), texture.GetAddressOf(), this->textureView.GetAddressOf());
		if (FAILED(hr))
		{
			this->Initialize1x1ColorTexture(device, ColorHelpers::UnloadedTextureColor, type);
		}
		return;
	}
	else
	{
		HRESULT hr = DirectX::CreateWICTextureFromFile(device, StringHelper::StringToWide(filePath).c_str(), texture.GetAddressOf(), this->textureView.GetAddressOf());
		if (FAILED(hr))
		{
			this->Initialize1x1ColorTexture(device, ColorHelpers::UnloadedTextureColor, type);
		}
		return;
	}
}

aiTextureType ModelTexture::GetType()
{
	return this->type;
}

ID3D11ShaderResourceView* ModelTexture::GetTextureResourceView()
{
	return this->textureView.Get();
}

ID3D11ShaderResourceView** ModelTexture::GetTextureResourceViewAddress()
{
	return this->textureView.GetAddressOf();
}

void ModelTexture::Initialize1x1ColorTexture(ID3D11Device* device, const ColorHelper& colorData, aiTextureType type)
{
	InitializeColorTexture(device, &colorData, 1, 1, type);
}

void ModelTexture::InitializeColorTexture(ID3D11Device* device, const ColorHelper* colorData, UINT width, UINT height, aiTextureType type)
{
	this->type = type;
	CD3D11_TEXTURE2D_DESC textureDesc(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
	ID3D11Texture2D* p2DTexture = nullptr;
	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = colorData;
	initialData.SysMemPitch = width * sizeof(ColorHelper);
	HRESULT hr = device->CreateTexture2D(&textureDesc, &initialData, &p2DTexture);
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"Failed to initialize texture from color data.", L"Error", hr);
	}
	texture = static_cast<ID3D11Texture2D*>(p2DTexture);
	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);
	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, textureView.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"Failed to create shader resource view from texture generated from color data.", L"Error", hr);
	}
}