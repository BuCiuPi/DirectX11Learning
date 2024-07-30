#ifndef D3DApp_H
#define D3DApp_H

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "D3DUtil.h"
#include "GameTimer.h"

#include <string>

using namespace DirectX;


class D3DApp
{


public:
	D3DApp(HINSTANCE hInstance);

	HINSTANCE               g_hInst = nullptr;
	HWND                    g_hWnd = nullptr;
	int mClientWidth;
	int mClientHeight;
	bool mAppPaused;
	bool mMinimized;
	bool mMaximized;
	bool mResizing;

	bool mEnable4xMsaa;
	UINT m4xMsaaQuality;


	D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device* g_pd3dDevice = nullptr;
	ID3D11Device1* g_pd3dDevice1 = nullptr;
	ID3D11DeviceContext* g_pImmediateContext = nullptr;
	ID3D11DeviceContext1* g_pImmediateContext1 = nullptr;
	D3D11_VIEWPORT g_pSceneViewport;

	IDXGISwapChain* g_pSwapChain = nullptr;
	IDXGISwapChain1* g_pSwapChain1 = nullptr;

	ID3D11Texture2D* g_pDepthStencilBuffer;
	ID3D11DepthStencilView* g_pDepthStencilView;

	ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
	ID3D11VertexShader* g_pVertexShader = nullptr;
	ID3D11PixelShader* g_pPixelShader = nullptr;
	ID3D11InputLayout* g_pVertexLayout = nullptr;
	ID3D11Buffer* g_pVertexBuffer = nullptr;
	ID3D11Buffer* g_pIndexBuffer = nullptr;
	ID3D11Buffer* g_pConstantBuffer = nullptr;

	XMMATRIX                g_World;
	XMMATRIX                g_View;
	XMMATRIX                g_Projection;

	GameTimer mTimer;

	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	virtual void OnResize();
	void CleanupDevice();
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	virtual void UpdateScene(float dt)=0;
	virtual void DrawScene()=0;

	bool Init(int nShowCmd);
	int Run();

	float     AspectRatio()const;

	void UpdateApplicationLoop();

	// Convenience overrides for handling mouse input.
	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }
protected:

	void CalculateFrameStarts();
	std::wstring mMainWndCaption;

	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	UINT mIndexCount;
	UINT mVertexCount;
private:


};

#endif // !D3DApp_H

