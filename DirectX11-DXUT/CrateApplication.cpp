#include "CrateApplication.h"
#include "GeometryGenerator.h"
#include "DDSTextureLoader.h"
#include "D3DUtil.h"

CrateApplication::CrateApplication(HINSTANCE hInstance) : DirectX11Application(hInstance)
{
	mCamera.SetPosition(XMFLOAT3(0.0f, 5.0f, -4.0f));

	// Directional light.
	mDirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	// Point light--position is changed every frame to animate in UpdateScene function.
	mPointLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mPointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mPointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mPointLight.Attenuation = XMFLOAT3(0.0f, 0.1f, 0.0f);
	mPointLight.Range = 25.0f;

	// Spot light--position and direction changed every frame to animate in UpdateScene function.
	mSpotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mSpotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	mSpotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSpotLight.Attenuation = XMFLOAT3(1.0f, 0.0f, 0.0f);
	mSpotLight.Spot = 96.0f;
	mSpotLight.Range = 10000.0f;

	mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
}

void CrateApplication::DrawScene()
{
	RotateBox();

	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, reinterpret_cast<const float*>(&Colors::MidnightBlue));
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//g_pImmediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	PerFrameBuffer pfb;
	pfb.gDirLight = mDirLight;
	pfb.gPointLight = mPointLight;
	pfb.gSpotLight = mSpotLight;
	//pfb.gEyePosW = mEyePosW;
	g_pImmediateContext->UpdateSubresource(mPerFrameBuffer, 0, nullptr, &pfb, 0, 0);
	g_pImmediateContext->PSSetConstantBuffers(1, 1, &mPerFrameBuffer);


	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(mCamera.View());
	cb.mProjection = XMMatrixTranspose(mCamera.Proj());
	XMVECTOR detBox = XMMatrixDeterminant(g_World);
	cb.mWorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(&detBox,g_World));
	cb.gMaterial = mBoxMat;


	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->PSSetShaderResources(0, 1, &mTexture);
	g_pImmediateContext->PSSetSamplers(0, 1, &mSamplerLinear);

	g_pImmediateContext->DrawIndexed(mIndexCount, 0, 0);

	g_pSwapChain->Present(0, 0);
}

void CrateApplication::RotateBox()
{
	static float t = 0.0f;
	if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static ULONGLONG timeStart = 0;
		ULONGLONG timeCur = GetTickCount64();
		if (timeStart == 0)
			timeStart = timeCur;
		t = (timeCur - timeStart) / 1000.0f;
	}

	//
	// Animate the cube
	//
	g_World = XMMatrixRotationY(t);
}

void CrateApplication::BuildGeometryBuffer()
{
	MeshData box;
	GeometryGenerator GeoGen;
	GeoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

	mVertexCount = box.Vertices.size();
	mIndexCount = box.Indices.size();

	std::vector<SimpleVertex> vertices(mVertexCount);
	for (INT i = 0; i < mVertexCount; i++)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].Tex = box.Vertices[i].TexC;

		float colVal = (float)i / (mVertexCount - 1);
		vertices[i].Color = XMFLOAT4(colVal, 1 - colVal, colVal, 1.0f);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * mVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(g_pd3dDevice->CreateBuffer(&vbd, &vinitData, &g_pVertexBuffer));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &box.Indices[0];
	HR(g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &g_pIndexBuffer));
}

void CrateApplication::BuildFX()
{
	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"Texture.fxh", "VS", "vs_4_0", &pVSBlob);
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

	InputLayouts::BuildVertexLayout(g_pd3dDevice, pVSBlob, InputLayoutDesc::Basic32, ARRAYSIZE(InputLayoutDesc::Basic32), &g_pVertexLayout);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"Texture.fxh", "PS", "ps_4_0", &pPSBlob);
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

void CrateApplication::BuildConstantBuffer()
{
	DirectX11Application::BuildConstantBuffer();

	D3D11_BUFFER_DESC bd;

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PerFrameBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;
	HR(g_pd3dDevice->CreateBuffer(&bd, nullptr, &mPerFrameBuffer));

	HR(CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/WoodCrate01.dds", nullptr, &mTexture));

	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(g_pd3dDevice->CreateSamplerState(&sampDesc, &mSamplerLinear));
}

void CrateApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);

	XMFLOAT4 camPos;
	XMStoreFloat4(&camPos, mCamera.GetPositionXM());
	mEyePosW = camPos;

	mPointLight.Position.x = 2.0f * cosf(0.2f * mTimer.GetTotalTime());
	mPointLight.Position.z = 2.0f * sinf(0.2f * mTimer.GetTotalTime());
	mPointLight.Position.y = 10.0f;

	mSpotLight.Position = XMFLOAT3(mEyePosW.x, mEyePosW.y, mEyePosW.z);
	XMStoreFloat3(&mSpotLight.Direction, mCamera.GetLookXM());
}
