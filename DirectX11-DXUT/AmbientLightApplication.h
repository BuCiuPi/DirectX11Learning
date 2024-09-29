#ifndef AMBIENT_LIGHT_APPLICATION_H
#define AMBIENT_LIGHT_APPLICATION_H
#include "ConstantBuffer.h"
#include "DirectX11Application.h"

class GameObject;

struct ambientLightConstantBuffer
{
	XMFLOAT3 AmbientDown;
	float pad;
	XMFLOAT3 AmbientRange;
	float pad1;
};

class AmbientLightApplication : public DirectX11Application
{
public:
	AmbientLightApplication(HINSTANCE hInstance);


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


	ID3D11VertexShader* mNanoSuitVertexShader;
	ID3D11PixelShader* mNanoSuitPixelShader;
	ID3D11SamplerState* mSamplerLinear;

	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;
	ConstantBuffer<ambientLightConstantBuffer> mAmbientLightConstantBuffer;


	bool mIsWireFrame = false;
	float mLightRotationAngle;
};
#endif


