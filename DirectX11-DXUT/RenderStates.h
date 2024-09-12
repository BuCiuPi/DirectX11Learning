#ifndef RENDERSTATE_H
#define RENDERSTATE_H

#include "D3DUtil.h"
class RenderStates
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11RasterizerState* WireframeRS;
	static ID3D11RasterizerState* NoCullRS;
	static ID3D11RasterizerState* CullClockwiseRS;

	static ID3D11BlendState* AlphaToCoverageBS;
	static ID3D11BlendState* TransparentBS;
	static ID3D11BlendState* NoRenderTargetWritesBS;
	static ID3D11BlendState* AdditiveBlendingBS;

	static ID3D11DepthStencilState* MarkMirrorDSS;
	static ID3D11DepthStencilState* DrawReflectionDSS;
	static ID3D11DepthStencilState* NoDoubleBlendDSS;

	static ID3D11DepthStencilState* LessEqualDSS;
	static ID3D11DepthStencilState* NoDepthWrite;
	static ID3D11DepthStencilState* DisableDepth;


private:
	static void CreateRasterizerStates(ID3D11Device* device);
	static void CreateBlendStates(ID3D11Device* device);
	static void CreateDepthStencilStates(ID3D11Device* device);
};
#endif // !RENDERSTATE_H
