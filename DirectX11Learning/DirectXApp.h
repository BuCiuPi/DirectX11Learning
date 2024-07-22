#ifndef DIRECTXAPP_H
#define DIRECTXAPP_H

#include <string>
#include "DirectX3DUtil.h"
#include "GameTimer.h"

class DirectXApp
{
public:
	DirectXApp(HINSTANCE hInstance);
	~DirectXApp();

	HINSTANCE GetAppInstance() const;
	HWND MainWnd() const;
	float AspectRatio() const;

	int Run();
	void UpdateApplicationLoop();

	virtual bool Init();
	virtual void OnResize();
	virtual void UpdateScene(float DeltaTime) = 0;
	virtual void DrawScene() = 0;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);

protected:
	bool InitMainWindow();
	bool InitDirect3D();

	void CalculateFrameStats();

	HINSTANCE mhAppInstance;
	HWND mhMainWnd;
	bool mAppPaused;
	bool mMinimized;
	bool mMaximized;
	bool mResizing;

	UINT m4xMSAAQuality;

	GameTimer mTimer;

#pragma region DirectX Variable

	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* md3dImmediateContext;
	IDXGISwapChain* mSwapChain;
	ID3D11Texture2D* mDepthStencilBuffer;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11DepthStencilView* mDepthStencilView;
	D3D11_VIEWPORT mScreenViewPort;

	std::wstring mMainWndCaption;
	D3D_DRIVER_TYPE md3dDriverType;

	int mClientWidth;
	int mClientHeight;

	bool mEnable4xMSAA;

#pragma endregion
};

#endif // !DIRECTXAPP_H
