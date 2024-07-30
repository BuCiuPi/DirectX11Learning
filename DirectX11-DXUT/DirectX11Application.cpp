#include "DirectX11Application.h"
#include "MathHelper.h"
#include "D3DUtil.h"

DirectX11Application::DirectX11Application(HINSTANCE hInstance) : D3DApp(hInstance)
{
	mCurrentCameraPos = XMVectorSet(0.0f, 2.0f, -10.0f, 0.0f);
}

bool DirectX11Application::Init(int nShowCmd)
{
	if (!D3DApp::Init(nShowCmd))
	{
		return false;
	}

	BuildGeometryBuffer();
	BuildConstantBuffer();
	BuildFX();

	return true;
}

void DirectX11Application::OnResize()
{
	D3DApp::OnResize();

	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f);
}

void DirectX11Application::UpdateScene(float dt)
{
	// Initialize the world matrix
	g_World = XMMatrixIdentity();

	// Initialize the view matrix

	XMVECTOR Target = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	g_View = XMMatrixLookAtLH(XMVectorScale( mCurrentCameraPos, 0.3f), Target, Up);
}

void DirectX11Application::DrawScene()
{

	//
	// Clear the back buffer
	//
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	//g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//g_pImmediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//
	// Update variables
	//
	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(g_View);
	cb.mProjection = XMMatrixTranspose(g_Projection);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	//
	// Renders a triangle
	//
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->DrawIndexed(mIndexCount, 0, 0);        // 36 vertices needed for 12 triangles in a triangle list

	//
	// Present our back buffer to our front buffer
	//
	g_pSwapChain->Present(0, 0);
}

void DirectX11Application::OnMouseDown(WPARAM btnState, int x, int y)
{
	mMouseHolded = true;

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void DirectX11Application::OnMouseUp(WPARAM btnState, int x, int y)
{
	mMouseHolded = false;

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void DirectX11Application::OnMouseMove(WPARAM btnState, int x, int y)
{
	if (mMouseHolded)
	{
		//mCurrentCameraPos = XMVectorAdd(mCurrentCameraPos, XMVectorSet(x - mLastMousePos.x, 0.0f, 0.0f, 0.0f));
		mCurrentCameraPos = XMVectorAdd(mCurrentCameraPos, XMVectorSet(0.0f, 0.0f, y - mLastMousePos.y, 0.0f));
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void DirectX11Application::BuildConstantBuffer()
{
	D3D11_BUFFER_DESC bd;
	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	HR(g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer));
}

void DirectX11Application::BuildFX()
{
	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"color.fxh", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return;
	}

	hr = BuildVertexLayout(pVSBlob);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"color.fxh", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return;
}

HRESULT DirectX11Application::BuildVertexLayout(ID3DBlob* pVSBlob)
{
	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	HRESULT hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);

	pVSBlob->Release();

	return hr;
}