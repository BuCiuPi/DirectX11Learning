#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireframeRS = 0;
ID3D11RasterizerState* RenderStates::NoCullRS = 0;
ID3D11RasterizerState* RenderStates::CullClockwiseRS = 0;

ID3D11BlendState* RenderStates::AlphaToCoverageBS = 0;
ID3D11BlendState* RenderStates::TransparentBS = 0;
ID3D11BlendState* RenderStates::NoRenderTargetWritesBS = 0;

ID3D11DepthStencilState* RenderStates::MarkMirrorDSS = 0;
ID3D11DepthStencilState* RenderStates::DrawReflectionDSS = 0;
ID3D11DepthStencilState* RenderStates::NoDoubleBlendDSS = 0;

void RenderStates::InitAll(ID3D11Device* device)
{

	CreateRasterizerStates(device);
	CreateBlendStates(device);
	CreateDepthStencilStates(device);
}

void RenderStates::CreateDepthStencilStates(ID3D11Device* device)
{
	D3D11_DEPTH_STENCIL_DESC mirrorDesc;
	mirrorDesc.DepthEnable = true;
	mirrorDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	mirrorDesc.DepthFunc = D3D11_COMPARISON_LESS;
	mirrorDesc.StencilEnable = true;
	mirrorDesc.StencilReadMask = 0xff;
	mirrorDesc.StencilWriteMask = 0xff;

	mirrorDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	mirrorDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HR(device->CreateDepthStencilState(&mirrorDesc, &MarkMirrorDSS));

	D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
	drawReflectionDesc.DepthEnable = true;
	drawReflectionDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	drawReflectionDesc.DepthFunc = D3D11_COMPARISON_LESS;
	drawReflectionDesc.StencilEnable = true;
	drawReflectionDesc.StencilReadMask = 0xff;
	drawReflectionDesc.StencilWriteMask = 0xff;

	drawReflectionDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	drawReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&drawReflectionDesc, &DrawReflectionDSS));

	D3D11_DEPTH_STENCIL_DESC noDoubleBLendDesc;
	noDoubleBLendDesc.DepthEnable = true;
	noDoubleBLendDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	noDoubleBLendDesc.DepthFunc = D3D11_COMPARISON_LESS;
	noDoubleBLendDesc.StencilEnable = true;
	noDoubleBLendDesc.StencilReadMask = 0xff;
	noDoubleBLendDesc.StencilWriteMask = 0xff;

	noDoubleBLendDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBLendDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBLendDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBLendDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	noDoubleBLendDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBLendDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBLendDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBLendDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&noDoubleBLendDesc, &NoDoubleBlendDSS));
}

void RenderStates::CreateBlendStates(ID3D11Device* device)
{
	D3D11_BLEND_DESC alphaToCoverageDesc = { 0 };
	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&alphaToCoverageDesc, &AlphaToCoverageBS));

	D3D11_BLEND_DESC transparentDesc = { 0 };
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;

	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&transparentDesc, &TransparentBS));

	D3D11_BLEND_DESC noRenderTargetWritesDesc = { 0 };
	noRenderTargetWritesDesc.AlphaToCoverageEnable = false;
	noRenderTargetWritesDesc.IndependentBlendEnable = false;

	noRenderTargetWritesDesc.RenderTarget[0].BlendEnable = false;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	HR(device->CreateBlendState(&noRenderTargetWritesDesc, &NoRenderTargetWritesBS));
}

void RenderStates::CreateRasterizerStates(ID3D11Device* device)
{
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&wireframeDesc, &WireframeRS));

	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&noCullDesc, &NoCullRS));

	D3D11_RASTERIZER_DESC cullClockwiseDesc;
	ZeroMemory(&cullClockwiseDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
	cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
	cullClockwiseDesc.FrontCounterClockwise = true;
	cullClockwiseDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&cullClockwiseDesc, &CullClockwiseRS));
}

void RenderStates::DestroyAll()
{
	ReleaseCOM(WireframeRS);
	ReleaseCOM(NoCullRS);
	ReleaseCOM(CullClockwiseRS);

	ReleaseCOM(AlphaToCoverageBS);
	ReleaseCOM(TransparentBS);
	ReleaseCOM(NoRenderTargetWritesBS);

	ReleaseCOM(MarkMirrorDSS);
	ReleaseCOM(DrawReflectionDSS);
	ReleaseCOM(NoDoubleBlendDSS);
}
