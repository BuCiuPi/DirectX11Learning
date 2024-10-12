#ifndef PBR_SHADING_APPLICATILON_H
#define PBR_SHADING_APPLICATILON_H
#include "ConstantBuffer.h"
#include "DirectX11Application.h"

class ShaderMaterial;
class GameObject;

class PBRShadingApplication : public DirectX11Application
{
public:
	PBRShadingApplication(HINSTANCE hInstance);

	virtual bool Init(int nShowCmd) override;

	virtual void DrawScene()override;
	virtual void UpdateScene(float dt)override;

	virtual void BuildGeometryBuffer() override;

	virtual void BuildConstantBuffer() override;

	virtual void CleanupDevice() override;

	virtual void BuildFX() override;
	void BuildNanoSuitFX();

private:
	Sky* mSky;

	GameObject* mNanoSuitGameObject;

	Material mMaterial;

	ID3D11ShaderResourceView* mNanoSuitTexture;

	ID3D11InputLayout* mInputLayout;

	ShaderMaterial* mShaderMaterial;

	ID3D11SamplerState* mSamplerLinear;

	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;
	ConstantBuffer<WavePerFrameBuffer> mPerFrameBuffer;

	ConstantBuffer<FrameBufferType> cbFrameBuffer;
	ConstantBuffer<ObjectBufferType> cbObjectBuffer;

	DirectionalLight mDirLights[3];
	bool mIsWireFrame;
	float mLightRotationAngle;
};
#endif


