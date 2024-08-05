#include "LitApplication.h"
#include "GeometryGenerator.h"

LitApplication::LitApplication(HINSTANCE hInstance) : DirectX11Application(hInstance)
{
	mCamera.Position = XMVectorSet(0.0f, 15.0f, -50.0f, 0.0f);

	XMMATRIX I = XMMatrixIdentity();

	XMStoreFloat4x4(&mGridWorld, XMMatrixMultiply(I, XMMatrixScaling(20.0f, 1.0f, 20.0f)));

	XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMMATRIX boxWorld = XMMatrixMultiply(boxScale, boxOffset);
	XMStoreFloat4x4(&mBoxWorld, boxWorld);

	XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&mCenterSphere, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

		XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
	}

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

	mGridMat.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	mGridMat.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	mGridMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mSphereMat.Ambient = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	mSphereMat.Diffuse = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	mSphereMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);

}

void LitApplication::DrawScene()
{
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

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



	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(g_View);
	cb.mProjection = XMMatrixTranspose(g_Projection);


	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);


	cb.mWorld = XMLoadFloat4x4(&mGridWorld);
	XMVECTOR detGrid = XMMatrixDeterminant(XMLoadFloat4x4(&mGridWorld));
	cb.mWorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(&detGrid, XMLoadFloat4x4(&mGridWorld)));
	cb.gMaterial = mGridMat;

	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mBoxWorld));

	XMVECTOR detBox = XMMatrixDeterminant(XMLoadFloat4x4(&mBoxWorld));
	cb.mWorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(&detBox, XMLoadFloat4x4(&mBoxWorld)));
	cb.gMaterial = mSphereMat;

	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);



	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mCenterSphere));
	XMVECTOR detSphere = XMMatrixDeterminant(XMLoadFloat4x4(&mCenterSphere));
	cb.mWorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(&detSphere, XMLoadFloat4x4(&mCenterSphere)));
	cb.gMaterial = mSphereMat;

	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);

	for (int i = 0; i < 10; ++i)
	{
		cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mCylWorld[i]));
		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
		g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

		g_pImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);

		cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mSphereWorld[i]));
		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
		g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

		g_pImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
	}

	g_pSwapChain->Present(0, 0);
}

void LitApplication::BuildGeometryBuffer()
{
	MeshData box;
	MeshData grid;
	MeshData sphere;
	MeshData cylinder;

	GeometryGenerator geoGen;

	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	//geoGen.CreateHeartPlane2D(grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	mBoxVertexCount = box.Vertices.size();
	mGridVertexCount = grid.Vertices.size();
	mSphereVertexCount = sphere.Vertices.size();
	mCylinderVertexCount = cylinder.Vertices.size();

	mBoxVertexOffset = 0;
	mGridVertexOffset = mBoxVertexCount;
	mSphereVertexOffset = mGridVertexOffset + mGridVertexCount;
	mCylinderVertexOffset = mSphereVertexOffset + mSphereVertexCount;

	mBoxIndexCount = box.Indices.size();
	mGridIndexCount = grid.Indices.size();
	mSphereIndexCount = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	mBoxIndexOffset = 0;
	mGridIndexOffset = mBoxIndexCount;
	mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

	UINT totalVertexCount = mBoxVertexCount + mGridVertexCount + mSphereVertexCount + mCylinderVertexCount;
	UINT totalIndexCount = mBoxIndexCount + mGridIndexCount + mSphereIndexCount + mCylinderIndexCount;

	std::vector<SimpleVertex> vertices(totalVertexCount);

	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	UINT k = 0;
	for (UINT i = 0; i < mBoxVertexCount; i++)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		float colVal = (float)i / (mBoxVertexCount - 1);
		vertices[k].Color = XMFLOAT4(0.0f, colVal, 0.0f, 1.0f);
		vertices[k].Normal = box.Vertices[i].Normal;
		k++;
	}
	for (UINT i = 0; i < mGridVertexCount; i++)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		float colVal = (float)i / (mGridVertexCount - 1);
		vertices[k].Color = XMFLOAT4(0.0f, colVal, 0.0f, 1.0f);
		vertices[k].Normal = grid.Vertices[i].Normal;
		k++;
	}
	for (UINT i = 0; i < mSphereVertexCount; i++)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		float colVal = (float)i / (mSphereVertexCount - 1);
		vertices[k].Color = XMFLOAT4(0.0f, colVal, 0.0f, 1.0f);
		vertices[k].Normal = sphere.Vertices[i].Normal;
		k++;
	}
	for (UINT i = 0; i < mCylinderVertexCount; i++)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		float colVal = (float)i / (mCylinderVertexCount - 1);
		vertices[k].Color = XMFLOAT4(0.0f, colVal, 0.0f, 1.0f);
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		k++;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(g_pd3dDevice->CreateBuffer(&vbd, &vinitData, &g_pVertexBuffer));

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &g_pIndexBuffer));

	mVertexCount = mSphereVertexCount;
	mIndexCount = mSphereIndexCount;
}

HRESULT LitApplication::BuildVertexLayout(ID3DBlob* pVSBlob)
{
	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	HRESULT hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);

	pVSBlob->Release();

	return hr;
}

void LitApplication::BuildFX()
{
	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"Lighting.fxh", "VS", "vs_4_0", &pVSBlob);
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
	hr = CompileShaderFromFile(L"Lighting.fxh", "PS", "ps_4_0", &pPSBlob);
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

void LitApplication::BuildConstantBuffer()
{
	DirectX11Application::BuildConstantBuffer();

	D3D11_BUFFER_DESC bd;
	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PerFrameBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	HR(g_pd3dDevice->CreateBuffer(&bd, nullptr, &mPerFrameBuffer));
}

void LitApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);

	XMFLOAT4 camPos;
	XMStoreFloat4(&camPos, mCamera.Position);
	mEyePosW = camPos;

	mPointLight.Position.x = 20.0f * cosf(0.2f * mTimer.GetTotalTime());
	mPointLight.Position.z = 20.0f * sinf(0.2f * mTimer.GetTotalTime());
	mPointLight.Position.y = 1.0f;

	mSpotLight.Position = XMFLOAT3(mEyePosW.x, mEyePosW.y, mEyePosW.z);
	XMStoreFloat3(&mSpotLight.Direction, mCamera.GetViewDirection());
}
