#ifndef GEOMETRY_GENERATOR_H
#define GEOMETRY_GENERATOR_H

#include "D3DUtil.h"
#include "DirectXMath.h"
#include "windowsx.h"
#include <vector>

using namespace DirectX;
struct GeoVertex
{
	GeoVertex() {}
	GeoVertex(
		float px, float py, float pz,
		float nx, float ny, float nz,
		float tx, float ty, float tz,
		float u, float v)
		: Position(px, py, pz), Normal(nx, ny, nz),
		TangentU(tx, ty, tz), TexC(u, v) {}


	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT3 TangentU;
	XMFLOAT2 TexC;
};

struct MeshData
{
	std::vector<GeoVertex> Vertices;
	std::vector<UINT> Indices;
};

class GeometryGenerator
{
public:
	GeometryGenerator();
	void CreateCube(MeshData& meshData, UINT& vertexCount, UINT& indexCount);

	void CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData);
	void CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height,
		UINT sliceCount, UINT stackCount, MeshData& meshData);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height,
		UINT sliceCount, UINT stackCount, MeshData& meshData);

	void CreateBox(float width, float height, float depth, MeshData& meshData);
	void CreateSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData);
	void CreateHeartPlane2D(MeshData& meshData);

	void CreateFullscreenQuad(MeshData& meshData);


private:
};

#endif // !GEOMETRY_GENERATOR_H
