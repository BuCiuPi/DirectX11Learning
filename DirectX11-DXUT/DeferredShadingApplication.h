#ifndef DEFERRED_SHADING_APPLICATION_H
#define DEFERRED_SHADING_APPLICATION_H
#include "CGBuffer.h"
#include "CLightManager.h"
#include "CSceneManager.h"
#include "DirectX11Application.h"
#include "GameObject.h"
#include "ShaderMaterial.h"

class DeferredShadingApplication : public DirectX11Application
{
public:
	DeferredShadingApplication(HINSTANCE hInstance);

	virtual bool Init(int nShowCmd) override;

	virtual void DrawScene()override;
	virtual void UpdateScene(float dt)override;

	virtual void BuildGeometryBuffer() override;

	virtual void BuildConstantBuffer() override;

	virtual void CleanupDevice() override;

	virtual void BuildFX() override;
	void BuildNanoSuitFX();

	void InitScene();
private:
	GameObject* mNanoSuitGameObject;

	ID3D11VertexShader* mNanoSuitVertexShader;
	ID3D11PixelShader* mNanoSuitPixelShader;
	ID3D11SamplerState* mSamplerLinear;

	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;

	ID3D11ShaderResourceView* mStarCubeMap;

	CGBuffer mCgBuffer;
	CSceneManager mSceneManager;
	CLightManager mLightManager;


	bool mIsWireFrame = false;
	float mTotalTime = 0.0f;
	float mLightRotationAngle = 0.0f;

	ShaderMaterial* mShaderMaterial;
	ID3D11InputLayout* mInputLayout;
	Material mMaterial;
};
#endif


