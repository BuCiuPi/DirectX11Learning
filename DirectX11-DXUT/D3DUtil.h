#pragma once
#include <d3d11_1.h>
#include "d3dcompiler.h"
#include <directxcolors.h>
#include "DirectXMath.h"
#include "dxerr.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "DXUT.h"
#include "DDSTextureLoader.h"
//---------------------------------------------------------------------------------------
// Simple d3d error checker for book demos.
//---------------------------------------------------------------------------------------

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)												   \
	{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
		{                                                      \
			DXTrace(__FILEW__, (DWORD)__LINE__, hr, L#x, true); \
		}													   \
	}

#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 


//---------------------------------------------------------------------------------------
// Convenience macro for releasing COM objects.
//---------------------------------------------------------------------------------------

#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }

//---------------------------------------------------------------------------------------
// Convenience macro for deleting objects.
//---------------------------------------------------------------------------------------

#define SafeDelete(x) { delete x; x = 0; }

//---------------------------------------------------------------------------------------
// Utility classes.
//---------------------------------------------------------------------------------------

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
	XMFLOAT4 Color;
};


struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMMATRIX mWorldInvTranspose;
	Material gMaterial;
};

struct WaveConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMMATRIX mWorldInvTranspose;
	XMMATRIX gTexTransform;
	Material gMaterial;
};

struct PerFrameBuffer
{
	DirectionalLight gDirLight;
	PointLight gPointLight;
	SpotLight gSpotLight;
	XMFLOAT4 gEyePosW;
};

struct WavePerFrameBuffer
{
	DirectionalLight gDirLights[3];
	XMFLOAT4 gEyePosW;
};

namespace Vertex
{
	// Basic 32-byte vertex structure.
	struct Basic32
	{
		Basic32() : Pos(0.0f, 0.0f, 0.0f), Normal(0.0f, 0.0f, 0.0f), Tex(0.0f, 0.0f) {}
		Basic32(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT2& uv)
			: Pos(p), Normal(n), Tex(uv) {}
		Basic32(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
			: Pos(px, py, pz), Normal(nx, ny, nz), Tex(u, v) {}
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
	};


	struct TreePointSprite
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};
}

class InputLayoutDesc
{
public:
	static const D3D11_INPUT_ELEMENT_DESC Basic32[3];
	static const D3D11_INPUT_ELEMENT_DESC TreePointSprite[2];

};

class InputLayouts
{
public:
	static HRESULT BuildVertexLayout(ID3D11Device* device, ID3DBlob* pVSBlob,const D3D11_INPUT_ELEMENT_DESC layout[], UINT numElements, ID3D11InputLayout** inputLayout);
	static void DestroyAll();

	static ID3D11InputLayout* Basic32;
	static ID3D11InputLayout* TreePointSprite;
};

HRESULT LoadTextureArray(ID3D11DeviceContext* deviceContex, ID3D11Device* pd3dDevice, LPCTSTR* szTextureNames, int iNumTextures, ID3D11Texture2D** ppTex2D, ID3D11ShaderResourceView** ppSRV);
