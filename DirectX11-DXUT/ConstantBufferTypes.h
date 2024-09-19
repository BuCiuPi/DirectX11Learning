#ifndef CONSTANT_BUFFER_TYPES_H
#define CONSTANT_BUFFER_TYPES_H
#include <DirectXMath.h>

#include "LightHelper.h"

struct CB_VS_vertexshader
{
	DirectX::XMMATRIX mat;
	Material material;
	DirectX::XMMATRIX gWorldViewProjTex;
};

struct CB_VS_vertexshader_NormalAndDepth
{
	DirectX::XMMATRIX gWorldView;
	DirectX::XMMATRIX gWorldInvTransposeView;
	DirectX::XMMATRIX gWorldViewProj;
	DirectX::XMMATRIX gTexTransform;
};

struct CB_PS_pixelshader
{
	DirectionalLight gDirLight[3];
	XMFLOAT3 gEyePosW;
};

#endif

