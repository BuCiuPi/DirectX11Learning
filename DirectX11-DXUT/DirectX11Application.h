#ifndef DIRECTX11APPLICATION_H
#define DIRECTX11APPLICATION_H

#include "D3DApp.h"

class DirectX11Application : public D3DApp
{
public:
	DirectX11Application(HINSTANCE hInstance);

	virtual bool Init(int nShowCmd);
	void OnResize();
	virtual void UpdateScene(float dt) override;
	void UpdateCameraState(float dt);

	virtual void DrawScene() override;

	virtual	void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

protected:
	virtual	void BuildGeometryBuffer() = 0;
	virtual void BuildConstantBuffer();
	virtual void BuildFX();

	POINT mLastMousePos;

	float mTheta;
	float mPhi;
	float mRadius;

	bool mIsCameraMoveable = true;

	Camera mCamera;
private:
};

#endif // !DIRECTX11APPLICATION_H
