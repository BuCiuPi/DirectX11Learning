#ifndef C_SCENE_MANAGER_H
#define C_SCENE_MANAGER_H

#include "D3DUtil.h"
#include "GameObject.h"
#include "ShaderMaterial.h"

struct cb_vs_perObject_gBuffer
{
	XMMATRIX m_world;
	XMMATRIX m_worldViewProjection;
};

struct cb_ps_perObject_gBuffer
{
	XMFLOAT3 m_vEyePosition;
	float m_fspecExp;
	float m_fSpectIntensity;
	float pad[3];
};

class CSceneManager
{
public:
	CSceneManager();
	~CSceneManager();

	HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void Deinit();

	void RenderScene(ID3D11DeviceContext* deviceContext, Camera* camera);
	void SetVisibleGameObject(std::vector<GameObject*> gameObjects);

private:

	std::vector<GameObject*> mGameObjects;

	ConstantBuffer<cb_vs_perObject_gBuffer> mVSConstantBuffer;
	ConstantBuffer<cb_ps_perObject_gBuffer> mPSConstantBuffer;

	ShaderMaterial* mShaderMaterial;
	ID3D11InputLayout* mInputLayout;
};

#endif


