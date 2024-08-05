#ifndef LIT_APPLICATION_H
#define LIT_APPLICATION_H

#include "DirectX11Application.h"
#include "LightHelper.h"

class LitApplication : public DirectX11Application
{
public:
	LitApplication(HINSTANCE hInstance);

	virtual void DrawScene() override;
	

	virtual void UpdateScene(float dt) override;

protected:
	virtual void BuildGeometryBuffer() override;
	virtual HRESULT BuildVertexLayout(ID3DBlob* pVSBlob) override;
	virtual void BuildFX() override;
	virtual void BuildConstantBuffer() override;

private:
	ID3D11Buffer* mPerFrameBuffer = nullptr;

	DirectionalLight mDirLight;
	PointLight mPointLight;
	SpotLight mSpotLight;

	Material mBoxMat;
	Material mGridMat;
	Material mSphereMat;
	Material mCylinderMat;

	UINT mBoxVertexCount;
	UINT mGridVertexCount;
	UINT mSphereVertexCount;
	UINT mCylinderVertexCount;

	UINT mBoxVertexOffset;
	UINT mGridVertexOffset;
	UINT mSphereVertexOffset;
	UINT mCylinderVertexOffset;

	UINT mBoxIndexOffset;
	UINT mGridIndexOffset;
	UINT mSphereIndexOffset;
	UINT mCylinderIndexOffset;

	UINT mBoxIndexCount;
	UINT mGridIndexCount;
	UINT mSphereIndexCount;
	UINT mCylinderIndexCount;

	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mCenterSphere;

	XMFLOAT4 mEyePosW;
};

#endif // !LIT_APPLICATION_H
