#include "CylinderApplication.h"
#include "GeometryGenerator.h"

CylinderApplication::CylinderApplication(HINSTANCE hInstance) : DirectX11Application(hInstance)
{
	mCamera.SetPosition(XMFLOAT3(0.0f, 10.0f, -10.0f));
}

void CylinderApplication::BuildGeometryBuffer()
{
	MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	mIndexCount = cylinder.Indices.size();
	mVertexCount = cylinder.Vertices.size();

	std::vector<SimpleVertex> vertices(mVertexCount);
	for (size_t i = 0; i < mVertexCount; ++i)
	{
		XMFLOAT3 p = cylinder.Vertices[i].Position;
		vertices[i].Pos = p;
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
	IInitData.pSysMem = &cylinder.Indices[0];
	HR(g_pd3dDevice->CreateBuffer(&ibd, &IInitData, &g_pIndexBuffer));
	
}

void CylinderApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);
}
