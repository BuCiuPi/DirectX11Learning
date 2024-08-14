#ifndef MIRROR_APPLICATION_H
#define MIRROR_APPLICATION_H

#include "DirectX11Application.h"

class MirrorApplication : public DirectX11Application
{
public:
	MirrorApplication(HINSTANCE hInstance);
	virtual bool Init(int nShowCmd) override;

	virtual void DrawScene()override;
	virtual void UpdateScene(float dt)override;

	virtual void BuildGeometryBuffer() override;
	void BuildRoomGeometryBuffer();
	void BuildSkullGeometryBuffer();

	virtual void BuildConstantBuffer() override;

	virtual void CleanupDevice() override;

	virtual void BuildFX() override;
private:
	ID3D11Buffer* mRoomVB = nullptr;

	ID3D11Buffer* mSkullVB = nullptr;
	ID3D11Buffer* mSkullIB = nullptr;

	ID3D11Buffer* mPerFrameBuffer = nullptr;
	ID3D11SamplerState* mSamplerLinear = nullptr;

	XMFLOAT4X4 mRoomWorld;
	XMFLOAT4X4 mSkullWorld;

	ID3D11ShaderResourceView* mFloorDiffuseMapSRV = nullptr;
	ID3D11ShaderResourceView* mWallDiffuseMapSRV = nullptr;
	ID3D11ShaderResourceView* mMirrorDiffuseMapSRV = nullptr;

	DirectionalLight mDirLights[3];

	Material mRoomMat;
	Material mSkullMat;
	Material mMirrorMat;
	Material mShadowMat;

	UINT mSkullIndexCount;
	XMFLOAT3 mSkullTranslation;

	UINT mBoxIndexCount;

	UINT mGridIndexCount;
	POINT mLastMousePos;

};

#endif // !MIRROR_APPLICATION_H
