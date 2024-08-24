#include "SphereApplication.h"

#include "GeometryGenerator.h"

SphereApplication::SphereApplication(HINSTANCE hInstance) : DirectX11Application(hInstance)
{
	mCamera.SetPosition(XMFLOAT3(0.0f, -3.0f, -10.0));
}

void SphereApplication::BuildGeometryBuffer()
{
	MeshData sphere;
	GeometryGenerator geoGen;

	geoGen.CreateSphere(2.0f, 6, 6, sphere);

	mIndexCount = sphere.Indices.size();
	mVertexCount = sphere.Vertices.size();

	std::vector<SimpleVertex> vertices(mVertexCount);
	for (UINT i = 0; i < mVertexCount; i++)
	{
		vertices[i].Pos = sphere.Vertices[i].Position;
		float colval = (float)i / (mVertexCount - 1);
		vertices[i].Color = XMFLOAT4(colval, 1 - colval, colval, 1.0f);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * mVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = &vertices[0];
	HR(g_pd3dDevice->CreateBuffer(&vbd, &vInitData, &g_pVertexBuffer));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mIndexCount;
	ibd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA IInitData;
	IInitData.pSysMem = &sphere.Indices[0];
	HR(g_pd3dDevice->CreateBuffer(&ibd, &IInitData, &g_pIndexBuffer));
}

void SphereApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);
}