#include "ShapesApplication.h"
#include "GeometryGenerator.h"
#include "DirectXMath.h"
ShapeApplication::ShapeApplication(HINSTANCE hInstance) : DirectX11Application(hInstance)
{
	mCurrentCameraPos = XMVectorSet(0.0f, 20.0f, -100.0f, 0.0f);

	XMMATRIX I = XMMatrixIdentity();

	XMStoreFloat4x4(&mGridWorld, I);

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
}

void ShapeApplication::DrawScene()
{

#if 1

	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);

	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);


	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(g_View);
	cb.mProjection = XMMatrixTranspose(g_Projection);

	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);


	cb.mWorld = XMLoadFloat4x4(&mGridWorld);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

	cb.mWorld = XMMatrixTranspose( XMLoadFloat4x4(&mBoxWorld));
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

	cb.mWorld = XMMatrixTranspose( XMLoadFloat4x4(&mCenterSphere));
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);

	for (int i = 0; i < 10; ++i)
	{
		cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mCylWorld[i]));
		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

		g_pImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);

		cb.mWorld = XMMatrixTranspose( XMLoadFloat4x4(&mSphereWorld[i]));
		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

		g_pImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
	}

	
	g_pSwapChain->Present(0, 0);

#else
	DirectX11Application::DrawScene();
#endif // 0

}

void ShapeApplication::BuildGeometryBuffer()
{
	MeshData box;
	MeshData grid;
	MeshData sphere;
	MeshData cylinder;

	GeometryGenerator geoGen;

	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
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
		vertices[k].Color = XMFLOAT4(colVal, 1-colVal, 0.0f, 1.0f);
		k++;
	}
	for (UINT i = 0; i < mGridVertexCount; i++)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		float colVal = (float)i / (mGridVertexCount - 1);
		vertices[k].Color = XMFLOAT4(colVal, 1-colVal, 0.0f, 1.0f);
		k++;
	}
	for (UINT i = 0; i < mSphereVertexCount; i++)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		float colVal = (float)i / (mSphereVertexCount - 1);
		vertices[k].Color = XMFLOAT4(colVal, 1-colVal, 0.0f, 1.0f);
		k++;
	}
	for (UINT i = 0; i < mCylinderVertexCount; i++)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		float colVal = (float)i / (mCylinderVertexCount - 1);
		vertices[k].Color = XMFLOAT4(colVal, 1-colVal, 0.0f, 1.0f);
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

void ShapeApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);
}


