#ifndef GEOMETRY_GENERATOR_H
#define GEOMETRY_GENERATOR_H

#include "D3DUtil.h"
#include "DirectXMath.h"
#include "windowsx.h"
#include <vector>

using namespace DirectX;
struct GeoVertex
{
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

	void CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData);
private:
};

#endif // !GEOMETRY_GENERATOR_H
