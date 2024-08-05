#ifndef CRATE_APPLICATION_H
#define CRATE_APPLICATION_H

#include "DirectX11Application.h"
class CrateApplication : public DirectX11Application
{
public:
	CrateApplication(HINSTANCE hInstance);

	virtual void DrawScene() override;

	void RotateBox();

	virtual void UpdateScene(float dt) override;
	virtual void BuildGeometryBuffer() override;

	virtual void BuildFX() override;
	virtual void BuildConstantBuffer() override;
	virtual HRESULT BuildVertexLayout(ID3DBlob* pVSBlob) override;

private:
	ID3D11Buffer* mPerFrameBuffer = nullptr;
	ID3D11ShaderResourceView* mTexture = nullptr;
	ID3D11SamplerState* mSamplerLinear = nullptr;

	Material mBoxMat;

	DirectionalLight mDirLight;

	PointLight mPointLight;
	SpotLight mSpotLight;

	XMFLOAT4 mEyePosW;
};

#endif // !CRATE_APPLICATION_H
