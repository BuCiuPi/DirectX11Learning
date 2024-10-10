#ifndef C_LIGHT_MANAGER_H
#define C_LIGHT_MANAGER_H

#include "CGBuffer.h"
#include "ConstantBuffer.h"
#include "D3DUtil.h"
#include "ShaderMaterial.h"

struct CB_DIRECTIONAL
{
	XMFLOAT3 vAmbientLower;
	float pad1;
	XMFLOAT3 vAmbientRange;
	float pad2;
	XMFLOAT3 vDirToLight;
	float pad3;
	XMFLOAT3 vDirectionalColor;
};

class CLightManager
{
public:
	CLightManager();
	~CLightManager();

	HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* deviceContex);
	void Deinit();

	void Update();

	void SetAmbient(const XMFLOAT3& vAmbientLowerColor, const XMFLOAT3& vAmbientUpperColor)
	{
		mAmbientLowerColor = vAmbientLowerColor;
		mAmbientUpperColor = vAmbientUpperColor;
	}

	void SetDirectional(const XMVECTOR& directionalDir, const XMFLOAT3 directionalColor)
	{
		XMStoreFloat3(&mDirectionalDir, XMVector3Normalize(directionalDir));
		mDirectionalColor = directionalColor;
	}

	void DoLighting(ID3D11DeviceContext* deviceContext, CGBuffer* gBuffer);
private:

	typedef enum LIGHT_TYPE
	{
		TYPE_POINT = 0,
		TYPE_SPOT,
		TYPE_CAPSULE
	};

	struct Light
	{
		LIGHT_TYPE eLightType;
		XMVECTOR vPosition;
		XMVECTOR vDirection;
		float fRange;
		float fLength;
		float fOuterAngle;
		float fInnerAngle;
		XMVECTOR vColor;
	};

	void DrawDirectionalLight(ID3D11DeviceContext* deviceContext);

	ID3D11VertexShader* mDirLightVertexShader;
	ID3D11PixelShader* mDirLightPixelShader;

	ShaderMaterial mShaderMaterial;

	ConstantBuffer<CB_DIRECTIONAL> mDirLightCB;

	ID3D11DepthStencilState* mNoDepthWriteLessStencilMaskState;

	XMFLOAT3 mAmbientLowerColor;
	XMFLOAT3 mAmbientUpperColor;

	XMFLOAT3 mDirectionalDir;
	XMFLOAT3 mDirectionalColor;
};
#endif


