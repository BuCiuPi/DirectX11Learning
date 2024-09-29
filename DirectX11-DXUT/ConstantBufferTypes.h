#ifndef CONSTANT_BUFFER_TYPES_H
#define CONSTANT_BUFFER_TYPES_H
#include <DirectXMath.h>

#include "LightHelper.h"

struct CB_VS_vertexshader
{
	DirectX::XMMATRIX gWorld;
	DirectX::XMMATRIX gWorldViewProj;

	Material material;
};

struct CB_PS_pixelshader
{
	DirectionalLight gDirLight[3];
	XMFLOAT3 gEyePosW;
};

#endif

