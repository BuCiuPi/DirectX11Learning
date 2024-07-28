#ifndef DIRECTX11APPLICATION_H
#define DIRECTX11APPLICATION_H

#include "D3DApp.h"

class DirectX11Application : public D3DApp
{
public:
	DirectX11Application(HINSTANCE hInstance);
	~DirectX11Application();

	bool Init(int nShowCmd);
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffer();
	void BuildFX();
	HRESULT BuildVertexLayout(ID3DBlob* pVSBlob);

	bool mMouseHolded = false;
	POINT mMousePosXY;
	POINT mLastMousePos;
};

#endif // !DIRECTX11APPLICATION_H
