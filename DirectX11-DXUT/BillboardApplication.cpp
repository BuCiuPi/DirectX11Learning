#include "BillboardApplication.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "D3DUtil.h"
#include "RenderStates.h"


LPCTSTR mTreeTextureNames[] =
{
	L"Textures\\tree0.dds",
	L"Textures\\tree1.dds",
	L"Textures\\tree2.dds",
	L"Textures\\tree3.dds",
};

BillboardApplication::BillboardApplication(HINSTANCE hInstance) : DirectX11Application(hInstance)
{
	mCamera.Position = XMVectorSet(0.0f, 150.0f, -250.0f, 0.0f);
	mRadius = 300.0f;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mLandWorld, I);
	XMStoreFloat4x4(&mWavesWorld, I);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixScaling(50.0f, 50.0f, 50.0f));
	g_View, I;
	g_World = I;

	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);

	XMMATRIX mBoxTexScale = XMMatrixScaling(1.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mBoxTexTransform, mBoxTexScale);

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mLandMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLandMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mWavesMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mWavesMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mWavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mTreeMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mTreeMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mTreeMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mWaterTexOffset = XMFLOAT2(0.0f, 0.0f);
}

bool BillboardApplication::Init(int nShowCmd)
{
	if (!D3DApp::Init(nShowCmd))
	{
		return false;
	}
	mWaves.Init(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

	BuildGeometryBuffer();
	BuildConstantBuffer();
	BuildBlendState();
	BuildFX();
	BuildTreeFX();

	return true;
}

void BillboardApplication::DrawScene()
{
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	float BlendFactor[]{ 0.0f, 0.0f, 0.0f, 0.0f };

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	WavePerFrameBuffer pfb;
	for (size_t i = 0; i < 3; i++)
	{
		pfb.gDirLights[i] = mDirLights[i];
	}
	pfb.gEyePosW = mEyePosW;
	g_pImmediateContext->UpdateSubresource(mPerFrameBuffer, 0, nullptr, &pfb, 0, 0);
	g_pImmediateContext->PSSetConstantBuffers(1, 1, &mPerFrameBuffer);
	g_pImmediateContext->GSSetConstantBuffers(1, 1, &mPerFrameBuffer);

	WaveConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(g_View);
	cb.mProjection = XMMatrixTranspose(g_Projection);
	XMVECTOR detBox = XMMatrixDeterminant(g_World);
	cb.mWorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(&detBox, g_World));

	g_pImmediateContext->PSSetSamplers(0, 1, &mSamplerLinear);

	DrawTreeSprite(cb);

	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->GSSetShader(nullptr, nullptr, 0);

	//
	 //Draw the Box.
	//
	g_pImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mBoxWorld));
	cb.gTexTransform = XMMatrixTranspose(XMLoadFloat4x4(&mBoxTexTransform));
	cb.gMaterial = mBoxMat;
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->PSSetShaderResources(0, 1, &mBoxMapSRV);

	g_pImmediateContext->RSSetState(mNoCullRS);
	g_pImmediateContext->DrawIndexed(mBoxIndexCount, 0, 0);
	g_pImmediateContext->RSSetState(0);

	//
	 //Draw the land.
	//
	g_pImmediateContext->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(mLandIB, DXGI_FORMAT_R32_UINT, 0);

	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mLandWorld));
	cb.gTexTransform = XMMatrixTranspose(XMLoadFloat4x4(&mGrassTexTransform));
	cb.gMaterial = mLandMat;
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->PSSetShaderResources(0, 1, &mGrassMapSRV);

	g_pImmediateContext->DrawIndexed(mLandIndexCount, 0, 0);

	//
	// Draw the waves.
	//
	g_pImmediateContext->IASetVertexBuffers(0, 1, &mWavesVB, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(mWavesIB, DXGI_FORMAT_R32_UINT, 0);

	//g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mWavesWorld));
	cb.gTexTransform = XMMatrixTranspose(XMLoadFloat4x4(&mWaterTexTransform));
	cb.gMaterial = mWavesMat;
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->PSSetShaderResources(0, 1, &mWavesMapSRV);
	g_pImmediateContext->OMSetBlendState(mTransparentBlendState, BlendFactor, 0xffffffff);
	g_pImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);
	g_pImmediateContext->OMSetBlendState(mTransparentBlendState, nullptr, 0xffffffff);

	g_pSwapChain->Present(0, 0);
}

void BillboardApplication::DrawTreeSprite(WaveConstantBuffer cb)
{
	g_pImmediateContext->VSSetShader(mTreeVSShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(mTreePSShader, nullptr, 0);
	g_pImmediateContext->GSSetShader(mTreeGSShader, nullptr, 0);

	g_pImmediateContext->IASetInputLayout(InputLayouts::TreePointSprite);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	UINT stride = sizeof(Vertex::TreePointSprite);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &mTreeSpritesVB, &stride, &offset);

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	cb.gTexTransform = XMMatrixTranspose(XMMatrixIdentity());
	cb.gMaterial = mTreeMat;
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->GSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->PSSetShaderResources(0, 1, &mTreeTextureMapArraySRV);

	g_pImmediateContext->OMSetBlendState(RenderStates::AlphaToCoverageBS, blendFactor, 0xffffffff);

	g_pImmediateContext->Draw(TreeCount, 0);
	g_pImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
}

void BillboardApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);

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

	XMMATRIX rotation = XMMatrixRotationY(t * 0.001f);
	rotation = XMMatrixMultiply(XMLoadFloat4x4(&mBoxWorld), rotation);

	XMStoreFloat4x4(&mBoxWorld, rotation);


	static float t_base = 0.0f;
	if ((mTimer.GetTotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		DWORD i = 5 + rand() % (mWaves.RowCount() - 10);
		DWORD j = 5 + rand() % (mWaves.ColumnCount() - 10);

		float r = MathHelper::RandF(1.0f, 2.0f);

		mWaves.Disturb(i, j, r);
	}

	mWaves.Update(dt);

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(g_pImmediateContext->Map(mWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	SimpleVertex* v = reinterpret_cast<SimpleVertex*>(mappedData.pData);
	for (UINT i = 0; i < mWaves.VertexCount(); ++i)
	{
		v[i].Pos = mWaves[i];
		v[i].Normal = mWaves.Normal(i);

		v[i].Tex.x = 0.5f + mWaves[i].x / mWaves.Width();
		v[i].Tex.y = 0.5f - mWaves[i].z / mWaves.Depth();
	}

	g_pImmediateContext->Unmap(mWavesVB, 0);

	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	// Translate texture over time.
	mWaterTexOffset.y += 0.05f * dt;
	mWaterTexOffset.x += 0.1f * dt;
	XMMATRIX wavesOffset = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);

	// Combine scale and translation.
	XMStoreFloat4x4(&mWaterTexTransform, wavesScale * wavesOffset);

	XMFLOAT4 camPos;
	XMStoreFloat4(&camPos, mCamera.Position);
	mEyePosW = camPos;
}

void BillboardApplication::BuildGeometryBuffer()
{
	MeshData grid;

	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	mLandIndexCount = grid.Indices.size();
	mIndexCount = mLandIndexCount;
	mVertexCount = grid.Vertices.size();

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  In addition, color the vertices based on their height so we have
	// sandy looking beaches, grassy low hills, and snow mountain peaks.
	//

	std::vector<SimpleVertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GetHeight(p.x, p.z);

		vertices[i].Pos = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
		vertices[i].Tex = grid.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(g_pd3dDevice->CreateBuffer(&vbd, &vinitData, &mLandVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mLandIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &mLandIB));

	BuildWaveBuffer();
}

void BillboardApplication::BuildWaveBuffer()
{
	// Create the vertex buffer.  Note that we allocate space only, as
// we will be updating the data every time step of the simulation.

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(SimpleVertex) * mWaves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(g_pd3dDevice->CreateBuffer(&vbd, 0, &mWavesVB));

	// Create the index buffer.  The index buffer is fixed, so we only
	// need to create and set once.

	std::vector<UINT> indices(3 * mWaves.TriangleCount()); // 3 indices per face

	// Iterate over each quad.
	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (DWORD j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &mWavesIB));

	BuildBoxBuffer();
}

void BillboardApplication::BuildBoxBuffer()
{
	MeshData box;

	GeometryGenerator geoGen;

	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

	mBoxIndexCount = box.Indices.size();

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  In addition, color the vertices based on their height so we have
	// sandy looking beaches, grassy low hills, and snow mountain peaks.
	//

	std::vector<SimpleVertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		XMFLOAT3 p = box.Vertices[i].Position;

		vertices[i].Pos = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
		vertices[i].Tex = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * box.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(g_pd3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mBoxIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &box.Indices[0];
	HR(g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));

	BuildTreeSpritesBuffer();
}

void BillboardApplication::BuildTreeSpritesBuffer()
{
	Vertex::TreePointSprite v[TreeCount];

	for (UINT i = 0; i < TreeCount; ++i)
	{
		float x = MathHelper::RandF(-35.0f, 35.0f);
		float z = MathHelper::RandF(-35.0f, 35.0f);
		float y = GetHeight(x, z);

		// Move tree slightly above land height.
		y += 10.0f;

		v[i].Pos = XMFLOAT3(x, y, z);
		v[i].Size = XMFLOAT2(24.0f, 24.0f);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::TreePointSprite) * TreeCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;
	HR(g_pd3dDevice->CreateBuffer(&vbd, &vinitData, &mTreeSpritesVB));
}


void BillboardApplication::BuildConstantBuffer()
{
	D3D11_BUFFER_DESC bd;
	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WaveConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	HR(g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer));

	D3D11_BUFFER_DESC pfb;

	pfb.Usage = D3D11_USAGE_DEFAULT;
	pfb.ByteWidth = sizeof(PerFrameBuffer);
	pfb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pfb.CPUAccessFlags = 0;
	pfb.MiscFlags = 0;
	pfb.StructureByteStride = 0;
	HR(g_pd3dDevice->CreateBuffer(&pfb, nullptr, &mPerFrameBuffer));

	HR(CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/grass.dds", nullptr, &mGrassMapSRV));
	HR(CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/water2.dds", nullptr, &mWavesMapSRV));
	HR(CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/WireFence.dds", nullptr, &mBoxMapSRV));
	HR(LoadTextureArray(g_pImmediateContext, g_pd3dDevice, mTreeTextureNames, sizeof(mTreeTextureNames) / sizeof(mTreeTextureNames[0]), &mTreeTexture, &mTreeTextureMapArraySRV));


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

void BillboardApplication::CleanupDevice()
{
	ReleaseCOM(mLandVB);
	ReleaseCOM(mLandIB);
	ReleaseCOM(mWavesVB);
	ReleaseCOM(mWavesIB);
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mTreeSpritesVB);

	ReleaseCOM(mGrassMapSRV);
	ReleaseCOM(mWavesMapSRV);
	ReleaseCOM(mBoxMapSRV);
	ReleaseCOM(mTreeSpritesVB);

	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

float BillboardApplication::GetHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

XMFLOAT3 BillboardApplication::GetHillNormal(float x, float z) const
{
	XMFLOAT3 n(-0.03f * z * cosf(0.1f * x), 1.0f, -0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));
	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);
	return n;
}

void BillboardApplication::BuildFX()
{
	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"Blending.fxh", "VS", "vs_4_0", &pVSBlob);
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
	hr = CompileShaderFromFile(L"Blending.fxh", "PS", "ps_4_0", &pPSBlob);
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

void BillboardApplication::BuildTreeFX()
{
	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"TreeSprite.fxh", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &mTreeVSShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return;
	}

	InputLayouts::BuildVertexLayout(g_pd3dDevice, pVSBlob, InputLayoutDesc::TreePointSprite, ARRAYSIZE(InputLayoutDesc::TreePointSprite), &InputLayouts::TreePointSprite);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return;
	}
	
	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"TreeSprite.fxh", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &mTreePSShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return;

	// Compile the Geometry shader
	ID3DBlob* pGSBlob = nullptr;
	hr = CompileShaderFromFile(L"TreeSprite.fxh", "GS", "gs_4_0", &pGSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &mTreeGSShader);
	pGSBlob->Release();
	if (FAILED(hr))
		return;

}

void BillboardApplication::BuildBlendState()
{
	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	HR(g_pd3dDevice->CreateRasterizerState(&noCullDesc, &mNoCullRS));

	D3D11_BLEND_DESC transparentDesc = { 0 };
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;

	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(g_pd3dDevice->CreateBlendState(&transparentDesc, &mTransparentBlendState));
}

