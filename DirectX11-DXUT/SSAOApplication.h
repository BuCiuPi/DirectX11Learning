#ifndef SSAO_APPLICATION_H
#define SSAO_APPLICATION_H

#include "ConstantBuffer.h"
#include "DirectX11Application.h"
#include "SSAO.h"

class GameObject;

class SSAOApplication :public DirectX11Application
{
public:
	SSAOApplication(HINSTANCE hInstance);


	virtual bool Init(int nShowCmd) override;
	virtual void OnResize() override;

	virtual void DrawScene()override;
	virtual void UpdateScene(float dt)override;

	virtual void BuildGeometryBuffer() override;

	virtual void BuildConstantBuffer() override;

	virtual void CleanupDevice() override;

	virtual void BuildFX() override;
	void BuildNanoSuitFX();
	void BuildDepthAndNormalFX();

	void DrawSceneToSSAONormalDepthMap();

private:
	Sky* mSky;
	SSAO* mSSAO = nullptr;

	GameObject* mNanoSuitGameObject;

	ID3D11ShaderResourceView* mNanoSuitTexture;

	ID3D11VertexShader* mNanoSuitVertexShader;
	ID3D11PixelShader* mNanoSuitPixelShader;
	ID3D11SamplerState* mSamplerLinear;

	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;
	ConstantBuffer<WavePerFrameBuffer> mPerFrameBuffer;

	ID3D11VertexShader* mDepthAndNormalVertexShader;
	ID3D11PixelShader* mDepthAndNormalPixelShader;

	DirectionalLight mDirLights[3];
	bool mIsWireFrame;
	float mLightRotationAngle;
};
#endif


