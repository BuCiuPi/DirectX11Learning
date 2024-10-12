#include "RenderTexture.h"

#include "D3DApp.h"
#include "D3DUtil.h"

RenderTexture::RenderTexture()
	: mRenderTargetTexture(NULL), mRenderTargetView(NULL), mShaderResourceView(NULL)
{
}

RenderTexture::~RenderTexture()
{
	ReleaseCOM(mShaderResourceView);
	ReleaseCOM(mRenderTargetTexture);
	ReleaseCOM(mRenderTargetView);
}

bool RenderTexture::Initialize(ID3D11Device* device, int width, int height, int mipMaps)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResouceViewDesc;

	mWidth = width;
	mHeight = height;
	
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = mipMaps;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	device->CreateTexture2D(&textureDesc, nullptr, &mRenderTargetTexture);

	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	device->CreateRenderTargetView(mRenderTargetTexture, &renderTargetViewDesc, &mRenderTargetView);

	shaderResouceViewDesc.Format = textureDesc.Format;
	shaderResouceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResouceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResouceViewDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(mRenderTargetTexture, &shaderResouceViewDesc, &mShaderResourceView);

	return true;
}

void RenderTexture::SetRenderTarget(D3DApp* app,  ID3D11DeviceContext* deviceContext) const
{
	if (!app->ResizeDepthBuffer(mWidth, mHeight))
	{
		return;
	}

	app->ResizeViewPort(mWidth, mHeight);

	deviceContext->OMSetRenderTargets(1, &mRenderTargetView, app->g_pDepthStencilView);
}

void RenderTexture::ClearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView,
                                      float red, float green, float blue, float alpha)
{
	float color[4];

	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	deviceContext->ClearRenderTargetView(mRenderTargetView, color);
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0.0f);
}


ID3D11ShaderResourceView* RenderTexture::GetSRV() const
{
	return mShaderResourceView;
}

ID3D11Texture2D* RenderTexture::GetTexture() const
{
	return mRenderTargetTexture;
}
