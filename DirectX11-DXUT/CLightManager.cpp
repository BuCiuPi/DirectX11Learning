#include "CLightManager.h"

#include "CGBuffer.h"

CLightManager::CLightManager()
	: mDirLightVertexShader(NULL), mDirLightPixelShader(NULL), mNoDepthWriteLessStencilMaskState(NULL)
{
}

CLightManager::~CLightManager()
{
}

HRESULT CLightManager::Init(ID3D11Device* device , ID3D11DeviceContext* deviceContex)
{
	HRESULT hr;

	mDirLightCB.Initialize(device, deviceContex);

	ID3DBlob* blob = mShaderMaterial.BuildShader(device, L"DeferredDirectionalLight.hlsl", VertexShader);
	mShaderMaterial.BuildShader(device, L"DeferredDirectionalLight.hlsl", PixelShader);

	D3D11_DEPTH_STENCIL_DESC descDepth;
	descDepth.DepthEnable = TRUE;
	descDepth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	descDepth.DepthFunc = D3D11_COMPARISON_LESS;
	descDepth.StencilEnable = TRUE;
	descDepth.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	descDepth.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	const D3D11_DEPTH_STENCILOP_DESC noSkyStencilOp = {
		D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_KEEP,
		D3D11_COMPARISON_EQUAL
	};

	descDepth.FrontFace = noSkyStencilOp;
	descDepth.BackFace = noSkyStencilOp;

	device->CreateDepthStencilState(&descDepth, &mNoDepthWriteLessStencilMaskState);

	return S_OK;
}

void CLightManager::Deinit()
{
	SafeDelete(mNoDepthWriteLessStencilMaskState);
}

void CLightManager::Update()
{

}

void CLightManager::DoLighting(ID3D11DeviceContext* deviceContext,CGBuffer* gBuffer)
{
	deviceContext->OMSetDepthStencilState(mNoDepthWriteLessStencilMaskState, 1);

	ID3D11ShaderResourceView* arrView[4] = {
		gBuffer->GetDepthView(),
		gBuffer->GetColorView(),
		gBuffer->GetNormalView(),
		gBuffer->GetSpecPowerView()
	};
	deviceContext->PSSetShaderResources(0, 4, arrView);

	DrawDirectionalLight(deviceContext);

	ZeroMemory(arrView, sizeof(arrView));
	deviceContext->PSSetShaderResources(0, 4, arrView);
}

void CLightManager::DrawDirectionalLight(ID3D11DeviceContext* deviceContext)
{
	mDirLightCB.data.vAmbientLower = mAmbientLowerColor;
	mDirLightCB.data.vAmbientRange = mAmbientUpperColor;
	mDirLightCB.data.vDirToLight = mDirectionalDir;
	mDirLightCB.data.vDirectionalColor = mDirectionalColor;
	mDirLightCB.ApplyChanges();
	mDirLightCB.PSShaderUpdate(1);

	deviceContext->IASetInputLayout(NULL);
	deviceContext->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	deviceContext->GSSetShader(NULL, NULL, 0);
	mShaderMaterial.SetShader(deviceContext);

	deviceContext->Draw(4, 0);

	ID3D11ShaderResourceView* arrRV[1] = { NULL };
	deviceContext->PSSetShaderResources(4, 1, arrRV);
	mShaderMaterial.UnSetShader(deviceContext);
	deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
