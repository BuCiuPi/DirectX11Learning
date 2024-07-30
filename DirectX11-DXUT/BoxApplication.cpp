#include "BoxApplication.h"
#include "MathHelper.h"
#include <sstream>
#include "GeometryGenerator.h"

BoxApplication::BoxApplication(HINSTANCE hInstance) : DirectX11Application(hInstance)
{
}

void BoxApplication::DrawScene()
{
	DirectX11Application::DrawScene();
	// Update our time
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

void BoxApplication::BuildGeometryBuffer()
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
