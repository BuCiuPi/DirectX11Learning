#ifndef SHAPE_APPLICATION_H
#define SHAPE_APPLICATION_H

#include "DirectX11Application.h"

class ShapeApplication : public DirectX11Application
{
public:
	ShapeApplication(HINSTANCE hInstance);

	virtual void DrawScene() override;

	virtual void BuildGeometryBuffer() override;

private:
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
};

#endif // !1
