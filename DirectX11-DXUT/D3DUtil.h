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
#include "RenderStates.h"
#include "Waves.h"
#include "Camera.h"
#include "Sky.h"
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


struct BasicConstantBuffer
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

struct ShadowMappingConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMMATRIX mWorldInvTranspose;
	XMMATRIX gTexTransform;
	XMMATRIX gShadowTransform;
	Material gMaterial;
};

struct SkyConstantBuffer
{
	XMMATRIX mMVP;
};

struct TerrainConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	Material gMaterial;
};

struct ParticleConstantBuffer
{
	XMFLOAT3 gAccelW;
	float fill;
};

struct PerFrameBuffer
{
	DirectionalLight gDirLight;
	PointLight gPointLight;
	SpotLight gSpotLight;
	XMFLOAT4 gEyePosW;
};

struct ParticlePerFrameBuffer
{
	XMMATRIX gViewProj;

	XMFLOAT3 gEyePosW;
	float gGameTime;

	XMFLOAT3 gEmitPosW;
	float gTimeStep;

	XMFLOAT3 gEmitDirW;
	float gFill;
};

struct WavePerFrameBuffer
{
	DirectionalLight gDirLights[3];
	XMFLOAT3 gEyePosW;
};

struct TerrainPerFrameBuffer
{
	DirectionalLight gDirLights[3];
	XMFLOAT3 gEyePosW;

	float gMinDist;
	float gMaxDist;

	float gMinTex;
	float gMaxTex;

	float gTexelCellSpaceU;
	float gTexelCellSpaceV;
	float gWorldCellSpace;
	XMFLOAT2 gTexScale;

	XMFLOAT4 gWorldFrustumPlanes[6];
};

struct ShadowConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMMATRIX gTexTransform;
};

struct InstancedData
{
	XMFLOAT4X4 World;
	XMFLOAT4 Color;
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

	struct Vertex
	{
		Vertex() {}
		Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), texCoord(u, v){}

		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 texCoord;

		float AmbientAccess;
	};


	struct TreePointSprite
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};

	struct PosNormalTexTan
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
		XMFLOAT3 TangentU;
	};

	struct Terrain
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Tex;
		XMFLOAT2 BoundsY;
	};

	struct Particle
	{
		XMFLOAT3 InitialPos;
		XMFLOAT3 InitialVel;
		XMFLOAT2 Size;
		float Age;
		unsigned int Type;
	};
}

class InputLayoutDesc
{
public:
	static const D3D11_INPUT_ELEMENT_DESC Basic32[3];
	static const D3D11_INPUT_ELEMENT_DESC Pos[1];
	static const D3D11_INPUT_ELEMENT_DESC TreePointSprite[2];
	static const D3D11_INPUT_ELEMENT_DESC InstancedBasic32[8];
	static const D3D11_INPUT_ELEMENT_DESC PosNormalTexTan[4];
	static const D3D11_INPUT_ELEMENT_DESC Terrain[3];
	static const D3D11_INPUT_ELEMENT_DESC Particle[5];
	static const D3D11_INPUT_ELEMENT_DESC NanoSuit[3];
	static const D3D11_INPUT_ELEMENT_DESC AmbientOcclusion[4];

};

class InputLayouts
{
public:
	static HRESULT BuildVertexLayout(ID3D11Device* device, ID3DBlob* pVSBlob, const D3D11_INPUT_ELEMENT_DESC layout[], UINT numElements, ID3D11InputLayout** inputLayout);
	static void DestroyAll();
	static ID3D11InputLayout* Basic32;
	static ID3D11InputLayout* Pos;
	static ID3D11InputLayout* TreePointSprite;
	static ID3D11InputLayout* PosNormalTexTan;
	static ID3D11InputLayout* Terrain;
	static ID3D11InputLayout* Particle;
	static ID3D11InputLayout* NanoSuit;
	static ID3D11InputLayout* AmbientOcclusion;
};

HRESULT LoadTextureArray(ID3D11DeviceContext* deviceContex, ID3D11Device* pd3dDevice, LPCTSTR* szTextureNames, int iNumTextures, ID3D11Texture2D** ppTex2D, ID3D11ShaderResourceView** ppSRV);

HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

ID3D11ShaderResourceView* CreateRandomTexture1DSRV(ID3D11Device* device);

static void ExtractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX m)
{
	XMFLOAT4X4 M;
	XMStoreFloat4x4(&M, m);
	//
	// Left
	//
	planes[0].x = M(0, 3) + M(0, 0);
	planes[0].y = M(1, 3) + M(1, 0);
	planes[0].z = M(2, 3) + M(2, 0);
	planes[0].w = M(3, 3) + M(3, 0);

	//
	// Right
	//
	planes[1].x = M(0, 3) - M(0, 0);
	planes[1].y = M(1, 3) - M(1, 0);
	planes[1].z = M(2, 3) - M(2, 0);
	planes[1].w = M(3, 3) - M(3, 0);

	//
	// Bottom
	//
	planes[2].x = M(0, 3) + M(0, 1);
	planes[2].y = M(1, 3) + M(1, 1);
	planes[2].z = M(2, 3) + M(2, 1);
	planes[2].w = M(3, 3) + M(3, 1);

	//
	// Top
	//
	planes[3].x = M(0, 3) - M(0, 1);
	planes[3].y = M(1, 3) - M(1, 1);
	planes[3].z = M(2, 3) - M(2, 1);
	planes[3].w = M(3, 3) - M(3, 1);

	//
	// Near
	//
	planes[4].x = M(0, 2);
	planes[4].y = M(1, 2);
	planes[4].z = M(2, 2);
	planes[4].w = M(3, 2);

	//
	// Far
	//
	planes[5].x = M(0, 3) - M(0, 2);
	planes[5].y = M(1, 3) - M(1, 2);
	planes[5].z = M(2, 3) - M(2, 2);
	planes[5].w = M(3, 3) - M(3, 2);

	// Normalize the plane equations.
	for (int i = 0; i < 6; ++i)
	{
		XMVECTOR v = XMPlaneNormalize(XMLoadFloat4(&planes[i]));
		XMStoreFloat4(&planes[i], v);
	}
}
