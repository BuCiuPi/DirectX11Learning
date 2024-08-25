#ifndef INSTANCING_N_CULLING_APPLICATION_H
#define INSTANCING_N_CULLING_APPLICATION_H

#include "DirectX11Application.h"
#include "DirectXCollision.h"



class InstancingAndCullingApplication : public DirectX11Application
{
public:
	InstancingAndCullingApplication(HINSTANCE hInstance);

	virtual bool Init(int nShowCmd) override;

	virtual void DrawScene()override;
	virtual void UpdateScene(float dt)override;
	virtual void OnResize() override;

	virtual void BuildGeometryBuffer() override;
	void BuildInstanceBuffer();

	virtual void BuildConstantBuffer() override;

	virtual void CleanupDevice() override;

	float GetHeight(float x, float z) const;

	XMFLOAT3 GetHillNormal(float x, float z) const;

	virtual void BuildFX() override;
private:
	ID3D11Buffer* mSkullVB = nullptr;
	ID3D11Buffer* mSkullIB = nullptr;
	ID3D11Buffer* mInstancedBuffer = nullptr;

	ID3D11Buffer* mPerFrameBuffer = nullptr;
	ID3D11SamplerState* mSamplerLinear = nullptr;

	BoundingBox mSkullBox;
	BoundingFrustum mCamFrustum;

	UINT mVisibleObjectCount = 0;

	std::vector<InstancedData> mInstancedData;

	XMFLOAT4X4 mSkullTexTransform;
	XMFLOAT4X4 mSkullWorld;

	ID3D11ShaderResourceView* mSkullSRV = nullptr;

	DirectionalLight mDirLights[3];

	Material mSkullMat;

	UINT mSkullIndexCount;

	bool mFrustumCullingEnabled = true;
};
#endif

