#include "MainWindow.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	MainWindow mainWnd(hInstance);

	if (!mainWnd.Init())
		return 0;

	return mainWnd.Run();
}


MainWindow::MainWindow(HINSTANCE hInstance) : DirectXApp(hInstance)
{
}

bool MainWindow::Init()
{
	if (!DirectXApp::Init())
	{
		return false;
	}
    return true;
}

void MainWindow::OnResize()
{
	DirectXApp::OnResize();
}

void MainWindow::UpdateScene(float dt)
{
}

void MainWindow::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Magenta));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	HR(mSwapChain->Present(0, 0));
}
