#ifndef NORMALMAPPING_APPLICATION_H
#define NORMALMAPPING_APPLICATION_H
#include "DirectX11Application.h"
class NormalMappingApplication : public DirectX11Application
{
public :
	NormalMappingApplication(HINSTANCE hInstance);
	virtual bool Init(int nShowCmd) override;

	virtual void DrawScene()override;
	virtual void UpdateScene(float dt)override;

	virtual void BuildGeometryBuffer() override;
	void BuildSkullGeometryBuffer();

	virtual void BuildConstantBuffer() override;

	virtual void CleanupDevice() override;

	virtual void BuildFX() override;

private:
	ID3D11VertexShader* mSkyVertexShader = nullptr;
	ID3D11PixelShader* mSkyPixelShader = nullptr;

	ID3D11Buffer* mSkyConstantBuffer = nullptr;

	Sky* mSky;

	ID3D11Buffer* mSkullVB = nullptr;
	ID3D11Buffer* mSkullIB = nullptr;

	ID3D11Buffer* mShapesVB = nullptr;
	ID3D11Buffer* mShapesIB = nullptr;

	ID3D11Buffer* mSkySphereVB = nullptr;
	ID3D11Buffer* mSkySphereIB = nullptr;

	ID3D11Buffer* mPerFrameBuffer = nullptr;
	ID3D11SamplerState* mSamplerLinear = nullptr;
	ID3D11SamplerState* mSamAnisotropic = nullptr;

	XMFLOAT4X4 mMeshTexTransform;
	XMFLOAT4X4 mMeshWorld;

	ID3D11ShaderResourceView* mFloorSRV = nullptr;
	ID3D11ShaderResourceView* mStoneSRV = nullptr;
	ID3D11ShaderResourceView* mBrickSRV = nullptr;

	ID3D11ShaderResourceView* mStoneNormalTexSRV = nullptr;
	ID3D11ShaderResourceView* mBrickNormalTexSRV = nullptr;

	ID3D11ShaderResourceView* nullSRV = nullptr;

	DirectionalLight mDirLights[3];

	Material mGridMat;
	Material mBoxMat;
	Material mCylinderMat;
	Material mSphereMat;
	Material mSkullMat;

	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mSkullWorld;


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

	UINT mSkullIndexCount;

	bool mFrustumCullingEnabled = true;

};

#endif // !NORMALMAPPING_APPLICATION_H


