#ifndef DIRECTX11APPLICATION_H
#define DIRECTX11APPLICATION_H

#include "D3DApp.h"
#include "Camera.h"

class DirectX11Application : public D3DApp
{
public:
	DirectX11Application(HINSTANCE hInstance);

	virtual bool Init(int nShowCmd);
	void OnResize();
	virtual void UpdateScene(float dt) override;
	virtual void DrawScene() override;

	virtual	void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

protected:
	virtual	void BuildGeometryBuffer() = 0;
	virtual void BuildConstantBuffer();
	virtual void BuildFX();
	virtual HRESULT BuildVertexLayout(ID3DBlob* pVSBlob);

	POINT mLastMousePos;

	bool mMouseHolded = false;

	Camera mCamera;
private:
};

#endif // !DIRECTX11APPLICATION_H
