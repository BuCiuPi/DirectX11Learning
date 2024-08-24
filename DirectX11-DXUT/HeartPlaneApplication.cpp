#include "HeartPlaneApplication.h"
#include "GeometryGenerator.h"

HeartPlaneApplication::HeartPlaneApplication(HINSTANCE hInstance) : DirectX11Application(hInstance)
{
	mCamera.SetPosition(XMFLOAT3(0.0f, 10.0f, -20.0f));
}

void HeartPlaneApplication::DrawScene()
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


	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(mCamera.View());
	cb.mProjection = XMMatrixTranspose(mCamera.Proj());

	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

	//cb.mWorld = XMMatrixTranspose(g_World);
	//g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	//g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	//g_pImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

	cb.mWorld = XMMatrixTranspose(g_World);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);


	g_pSwapChain->Present(0, 0);
}

void HeartPlaneApplication::BuildGeometryBuffer()
{
	MeshData box;
	MeshData heartPlane;

	GeometryGenerator geoGen;

	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateHeartPlane2D( heartPlane);

	mBoxVertexCount = box.Vertices.size();
	mGridVertexCount = heartPlane.Vertices.size();

	mBoxVertexOffset = 0;
	mGridVertexOffset = mBoxVertexCount;

	mBoxIndexCount = box.Indices.size();
	mGridIndexCount = heartPlane.Indices.size();

	mBoxIndexOffset = 0;
	mGridIndexOffset = mBoxIndexCount;

	UINT totalVertexCount = mBoxVertexCount + mGridVertexCount;
	UINT totalIndexCount = mBoxIndexCount + mGridIndexCount;

	std::vector<SimpleVertex> vertices(totalVertexCount);

	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	UINT k = 0;
	for (UINT i = 0; i < mBoxVertexCount; i++)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		float colVal = (float)i / (mBoxVertexCount - 1);
		vertices[k].Color = XMFLOAT4(colVal, 1 - colVal, 0.0f, 1.0f);
		k++;
	}
	for (UINT i = 0; i < mGridVertexCount; i++)
	{
		vertices[k].Pos = heartPlane.Vertices[i].Position;
		float colVal = (float)i / (mGridVertexCount - 1);
		vertices[k].Color = XMFLOAT4(colVal, 1 - colVal, 0.0f, 1.0f);
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
	indices.insert(indices.end(), heartPlane.Indices.begin(), heartPlane.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &g_pIndexBuffer));
}
