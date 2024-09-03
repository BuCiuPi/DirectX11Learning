#include "DirectX11Application.h"
#include "MathHelper.h"
#include "D3DUtil.h"
#include "Windows.h"

DirectX11Application::DirectX11Application(HINSTANCE hInstance) : D3DApp(hInstance)
, mCamera(XMFLOAT3(0.0f, 2.0f, -15.0f)), mTheta(1.5f * MathHelper::Pi), mPhi(0.42f * MathHelper::Pi), mRadius(50.0f)
{
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

	mCamera.SetLens(XM_PIDIV2, AspectRatio(), 0.01f, 1000.0f);
	mCamera.UpdateViewMatrix();
}


void DirectX11Application::UpdateScene(float dt)
{
	UpdateCameraState(dt);

	g_World = XMMatrixIdentity();
}

void DirectX11Application::UpdateCameraState(float dt)
{
	if (!mIsCameraMoveable)
	{
		return;	
	}

	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	//// Build the view matrix.
	//XMFLOAT3 pos = XMFLOAT3(x, y, z);

	//mCamera.SetPosition(pos);

	float camSpeed = mCamera.CameraSpeed;

	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		camSpeed *= 2;
		
	if (GetAsyncKeyState('W') & 0x8000)
		mCamera.Walk(camSpeed * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCamera.Walk(-camSpeed * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCamera.Strafe(-camSpeed * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCamera.Strafe(camSpeed * dt);

	if (GetAsyncKeyState('E') & 0x8000)
		mCamera.Fly(camSpeed * dt);

	if (GetAsyncKeyState('Q') & 0x8000)
		mCamera.Fly(-camSpeed * dt);

	mCamera.UpdateViewMatrix();
}

void DirectX11Application::DrawScene()
{
	//
	// Clear the back buffer
	//
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

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
	cb.mView = XMMatrixTranspose(mCamera.View());
	cb.mProjection = XMMatrixTranspose(mCamera.Proj());
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
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(g_hWnd);
}

void DirectX11Application::OnMouseUp(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	ReleaseCapture();
}

void DirectX11Application::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_RBUTTON) != 0)
	{
		mIsCameraMoveable = true;

		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		mCamera.Pitch(dy);
		mCamera.RotateY(dx);
	}
	else
	{
		mIsCameraMoveable = false;
	}

	//if ((btnState & MK_LBUTTON) != 0)
	//{
	//	// Make each pixel correspond to a quarter of a degree.
	//	float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
	//	float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

	//	// Update angles based on input to orbit camera around box.
	//	mTheta += dx;
	//	mPhi += dy;

	//	// Restrict the angle mPhi.
	//	mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	//}
	//else if ((btnState & MK_RBUTTON) != 0)
	//{
	//	// Make each pixel correspond to 0.01 unit in the scene.
	//	float dx = 0.01f * static_cast<float>(x - mLastMousePos.x);
	//	float dy = 0.01f * static_cast<float>(y - mLastMousePos.y);

	//	// Update the camera radius based on input.
	//	mRadius += dx - dy;

	//	// Restrict the radius.
	//	mRadius = MathHelper::Clamp(mRadius, 3.0f, 1000.0f);
	//}

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

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
	InputLayouts::BuildVertexLayout(g_pd3dDevice, pVSBlob, layout, numElements, &g_pVertexLayout);
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