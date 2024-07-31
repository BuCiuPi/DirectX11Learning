#ifndef HEART_APPLICATION_H
#define HEART_APPLICATION_H

#include "DirectX11Application.h"

class HeartPlaneApplication : public DirectX11Application
{
public:
	HeartPlaneApplication(HINSTANCE hInstance);


	virtual void DrawScene() override;

	virtual void BuildGeometryBuffer() override;

private:
	UINT mBoxVertexCount;
	UINT mGridVertexCount;

	UINT mBoxVertexOffset;
	UINT mGridVertexOffset;

	UINT mBoxIndexOffset;
	UINT mGridIndexOffset;

	UINT mBoxIndexCount;
	UINT mGridIndexCount;

	XMFLOAT4X4 mBoxWorld;

};

#endif // !HEART_APPLICATION_H
