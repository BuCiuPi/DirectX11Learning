#ifndef PICKING_APPLIATION_H

#define PICKING_APPLIATION_H
#include "DirectX11Application.h"
#include "DirectXCollision.h"

class PickingApplication : public DirectX11Application
{
public:
	PickingApplication(HINSTANCE hInstance);
	virtual bool Init(int nShowCmd) override;

	virtual void DrawScene()override;
	virtual void UpdateScene(float dt)override;
	virtual void OnResize() override;

	virtual	void OnMouseDown(WPARAM btnState, int x, int y) override;

	virtual void BuildGeometryBuffer() override;

	virtual void BuildConstantBuffer() override;

	virtual void CleanupDevice() override;

	float GetHeight(float x, float z) const;

	XMFLOAT3 GetHillNormal(float x, float z) const;

	virtual void BuildFX() override;

	void Pick(int sx, int sy);
private:
	ID3D11Buffer* mMeshVB = nullptr;
	ID3D11Buffer* mMeshIB = nullptr;

	ID3D11Buffer* mPerFrameBuffer = nullptr;
	ID3D11SamplerState* mSamplerLinear = nullptr;

	std::vector < Vertex::Basic32> mMeshVertices;
	std::vector < UINT> mMeshIndices;

	BoundingBox mMeshBox;
	BoundingFrustum mCamFrustum;


	XMFLOAT4X4 mMeshTexTransform;
	XMFLOAT4X4 mMeshWorld;

	ID3D11ShaderResourceView* mMeshSRV = nullptr;

	DirectionalLight mDirLights[3];

	Material mMeshMat;
	Material mPickedTriangleMat;

	UINT mMeshIndexCount;
	UINT mPickedTriangle;

	bool mFrustumCullingEnabled = true;
	
};

#endif
