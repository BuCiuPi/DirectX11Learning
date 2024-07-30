#include "HillApplication.h"
#include "GeometryGenerator.h"
#include "D3DUtil.h"
#include "MathHelper.h"
#include <string>
#include <sstream>

HillApplication::HillApplication(HINSTANCE hInstance) : DirectX11Application(hInstance)
{
	mCurrentCameraPos = XMVectorSet(0.0f, 100.0f, -50.0f, 0.0f);
}

void HillApplication::BuildGeometryBuffer()
{
	MeshData grid;

	GeometryGenerator geoGen;
	geoGen.CreateGrid(160.0f, 160.0f, 200, 200, grid);

	mIndexCount = grid.Indices.size();

	std::vector<SimpleVertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;

		// get height by using height equation
		p.y = GetHeight(p.x, p.z);

		vertices[i].Pos = p;

		if (p.y < -10.0f)
		{
			// Sandy beach color.
			vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		}
		else if (p.y < 5.0f)
		{
			// Light yellow-green.
			vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		}
		else if (p.y < 12.0f)
		{
			// Dark yellow-green.
			vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		}
		else if (p.y < 20.0f)
		{
			// Dark brown.
			vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			// White snow.
			vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * vertices.size();
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
	IInitData.pSysMem = &grid.Indices[0];
	HR(g_pd3dDevice->CreateBuffer(&ibd, &IInitData, &g_pIndexBuffer));
}

void HillApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);

	//// Update our time
	//static float t = 0.0f;
	//if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
	//{
	//	t += (float)XM_PI * 0.0125f;
	//}
	//else
	//{
	//	static ULONGLONG timeStart = 0;
	//	ULONGLONG timeCur = GetTickCount64();
	//	if (timeStart == 0)
	//		timeStart = timeCur;
	//	t = (timeCur - timeStart) / 1000.0f;
	//}

	////
	//// Animate the cube
	////
	//g_World = XMMatrixRotationY(t);

}

float HillApplication::GetHeight(float x, float z)const
{
	return 0.3f * (z * sinf(0.3f * x) + x * cosf(0.3f * z));
}
