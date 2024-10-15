#include "CGBuffer.h"

#pragma pack(push,1)
struct CB_GBUFFER_UNPACK
{
	XMFLOAT4 PerspectiveValues;
	XMMATRIX ViewInv;
};
#pragma pack(pop)


CGBuffer::CGBuffer()
	: m_pGBufferUnpackCB(NULL), m_DepthStencilRT(NULL), m_ColorSpecIntensityRT(NULL), m_NormalRT(NULL), m_SpecPowerRT(NULL)
	, m_DepthStencilDSV(NULL), m_DepthStencilReadOnlyDSV(NULL), m_ColorSpecIntensityRTV(NULL), m_NormalRTV(NULL), m_SpecPowerRTV(NULL)
	, m_DepthStencilSRV(NULL), m_ColorSpecIntensitySRV(NULL), m_NormalSRV(NULL), m_SpecPowerSRV(NULL), m_DepthStencilState(NULL)
{
}

CGBuffer::~CGBuffer()
{
}

HRESULT CGBuffer::Init(ID3D11Device* g_pd3dDevice, UINT width, UINT height)
{
	HRESULT hr;
	Deinit();

	// texture format
	static const DXGI_FORMAT depthStencilTextureFormat = DXGI_FORMAT_R24G8_TYPELESS;
	static const DXGI_FORMAT basicColorTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT normalTextureFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	static const DXGI_FORMAT specPowTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	// render view format
	static const DXGI_FORMAT depthStencilRenderViewFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	static const DXGI_FORMAT basicColorRenderViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT normalRenderViewFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	static const DXGI_FORMAT specPowRenderViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	// resource view format
	static const DXGI_FORMAT depthStencilResourceViewFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	static const DXGI_FORMAT basicColorResourceViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT normalResourceViewFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	static const DXGI_FORMAT specPowResourceViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D11_TEXTURE2D_DESC dtd;
	dtd.Width = width;
	dtd.Height = height;
	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = DXGI_FORMAT_UNKNOWN;
	dtd.SampleDesc.Count = 1;
	dtd.SampleDesc.Quality = 0;
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = 0;

	dtd.Format = depthStencilTextureFormat;
	g_pd3dDevice->CreateTexture2D(&dtd, NULL, &m_DepthStencilRT);

	dtd.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	dtd.Format = basicColorTextureFormat;
	g_pd3dDevice->CreateTexture2D(&dtd, NULL, &m_ColorSpecIntensityRT);

	dtd.Format = normalTextureFormat;
	g_pd3dDevice->CreateTexture2D(&dtd, NULL, &m_NormalRT);

	dtd.Format = specPowTextureFormat;
	g_pd3dDevice->CreateTexture2D(&dtd, NULL, &m_SpecPowerRT);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	dsvd.Format = depthStencilRenderViewFormat;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Texture2D.MipSlice = 0;
	dsvd.Flags = 0;

	g_pd3dDevice->CreateDepthStencilView(m_DepthStencilRT, &dsvd, &m_DepthStencilDSV);

	dsvd.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
	g_pd3dDevice->CreateDepthStencilView(m_DepthStencilRT, &dsvd, &m_DepthStencilReadOnlyDSV);

	D3D11_RENDER_TARGET_VIEW_DESC rtsvd;
	rtsvd.Format = basicColorRenderViewFormat;
	rtsvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtsvd.Texture2D.MipSlice = 0;

	g_pd3dDevice->CreateRenderTargetView(m_ColorSpecIntensityRT, &rtsvd, &m_ColorSpecIntensityRTV);

	rtsvd.Format = normalRenderViewFormat;
	g_pd3dDevice->CreateRenderTargetView(m_NormalRT, &rtsvd, &m_NormalRTV);

	rtsvd.Format = specPowRenderViewFormat;
	g_pd3dDevice->CreateRenderTargetView(m_SpecPowerRT, &rtsvd, &m_SpecPowerRTV);

	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
	dsrvd.Format = depthStencilResourceViewFormat;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	dsrvd.Buffer.FirstElement = 0;
	dsrvd.Buffer.NumElements = 0;
	dsrvd.Texture2D.MipLevels = 1;
	dsrvd.Texture2D.MostDetailedMip = 0;

	g_pd3dDevice->CreateShaderResourceView(m_DepthStencilRT, &dsrvd, &m_DepthStencilSRV);

	dsrvd.Format = basicColorResourceViewFormat;
	g_pd3dDevice->CreateShaderResourceView(m_ColorSpecIntensityRT, &dsrvd, &m_ColorSpecIntensitySRV);

	dsrvd.Format = normalResourceViewFormat;
	g_pd3dDevice->CreateShaderResourceView(m_NormalRT, &dsrvd, &m_NormalSRV);

	dsrvd.Format = specPowResourceViewFormat;
	g_pd3dDevice->CreateShaderResourceView(m_SpecPowerRT, &dsrvd, &m_SpecPowerSRV);

	D3D11_DEPTH_STENCIL_DESC descDepth;
	descDepth.DepthEnable = TRUE;
	descDepth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	descDepth.DepthFunc = D3D11_COMPARISON_LESS;

	descDepth.StencilEnable = TRUE;
	descDepth.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	descDepth.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	const D3D11_DEPTH_STENCILOP_DESC stencilMarkOp = { D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_REPLACE, D3D11_COMPARISON_ALWAYS };

	descDepth.FrontFace = stencilMarkOp;
	descDepth.BackFace = stencilMarkOp;

	g_pd3dDevice->CreateDepthStencilState(&descDepth, &m_DepthStencilState);

	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));

	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(CB_GBUFFER_UNPACK);
	g_pd3dDevice->CreateBuffer(&cbDesc, NULL, &m_pGBufferUnpackCB);

	return S_OK;
}

void CGBuffer::Deinit()
{
	SAFE_RELEASE(m_pGBufferUnpackCB);

	// Clear all allocated targets
	SAFE_RELEASE(m_DepthStencilRT);
	SAFE_RELEASE(m_ColorSpecIntensityRT);
	SAFE_RELEASE(m_NormalRT);
	SAFE_RELEASE(m_SpecPowerRT);

	// Clear all views
	SAFE_RELEASE(m_DepthStencilDSV);
	SAFE_RELEASE(m_DepthStencilReadOnlyDSV);
	SAFE_RELEASE(m_ColorSpecIntensityRTV);
	SAFE_RELEASE(m_NormalRTV);
	SAFE_RELEASE(m_SpecPowerRTV);
	SAFE_RELEASE(m_DepthStencilSRV);
	SAFE_RELEASE(m_ColorSpecIntensitySRV);
	SAFE_RELEASE(m_NormalSRV);
	SAFE_RELEASE(m_SpecPowerSRV);

	// Clear the depth stencil state
	SAFE_RELEASE(m_DepthStencilState);

}

void CGBuffer::PreRender(ID3D11DeviceContext* pd3dImmediateContext)
{
	pd3dImmediateContext->ClearDepthStencilView(m_DepthStencilDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pd3dImmediateContext->ClearRenderTargetView(m_ColorSpecIntensityRTV, clearColor);
	pd3dImmediateContext->ClearRenderTargetView(m_NormalRTV, clearColor);
	pd3dImmediateContext->ClearRenderTargetView(m_SpecPowerRTV, clearColor);

	ID3D11RenderTargetView* rt[3] = { m_ColorSpecIntensityRTV, m_NormalRTV, m_SpecPowerRTV };
	pd3dImmediateContext->OMSetRenderTargets(3, rt, m_DepthStencilDSV);
	pd3dImmediateContext->OMSetDepthStencilState(m_DepthStencilState, 1);
}

void CGBuffer::PostRender(ID3D11DeviceContext* pd3dImmediateContext)
{
	pd3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ID3D11RenderTargetView* rt[3] = { NULL, NULL, NULL };
	pd3dImmediateContext->OMSetRenderTargets(3, rt, m_DepthStencilReadOnlyDSV);
}

void CGBuffer::PrepareForUnpack(ID3D11DeviceContext* pd3dImmediateContext, Camera* g_Camera)
{
	HRESULT hr;

	// Fill the GBuffer unpack constant buffer
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	V(pd3dImmediateContext->Map(m_pGBufferUnpackCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	CB_GBUFFER_UNPACK* pGBufferUnpackCB = (CB_GBUFFER_UNPACK*)MappedResource.pData;
	const XMFLOAT4X4 pProj = g_Camera->Proj4x4();
	pGBufferUnpackCB->PerspectiveValues.x = 1.0f / pProj(0, 0);
	pGBufferUnpackCB->PerspectiveValues.y = 1.0f / pProj(1, 1);
	pGBufferUnpackCB->PerspectiveValues.z = pProj(3, 2);
	pGBufferUnpackCB->PerspectiveValues.w = -pProj(2, 2);

	XMVECTOR viewDet = XMMatrixDeterminant(g_Camera->View());
	XMMATRIX matViewInv = XMMatrixInverse(&viewDet, g_Camera->View());
	pGBufferUnpackCB->ViewInv = XMMatrixTranspose(matViewInv);

	pd3dImmediateContext->Unmap(m_pGBufferUnpackCB, 0);
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &m_pGBufferUnpackCB);

}
