#include "Octree.h"

Octree::Octree() : mRoot(0)
{
}

Octree::~Octree()
{
	SafeDelete(mRoot);
}

void Octree::Build(const std::vector<XMFLOAT3>& vertices, const std::vector<DWORD>& indices)
{
	mVertices = vertices;

	BoundingBox sceneBox = BuildAABB();

	mRoot = new OctreeNode();
	mRoot->Bounds = sceneBox;

	BuildOctree(mRoot, indices);
}

bool Octree::RayOctreeIntersect(FXMVECTOR rayPos, FXMVECTOR rayDir)
{
	return RayOctreeIntersect(mRoot, rayPos, rayDir);
}

BoundingBox Octree::BuildAABB()
{
	XMVECTOR vmin = XMVectorReplicate(+MathHelper::Infinity);
	XMVECTOR vmax = XMVectorReplicate(-MathHelper::Infinity);

	for (size_t i = 0; i < mVertices.size(); ++i)
	{
		XMVECTOR P = XMLoadFloat3(&mVertices[i]);

		vmin = XMVectorMin(vmin, P);
		vmax = XMVectorMax(vmax, P);
	}

	BoundingBox bounds;
	XMVECTOR C = 0.5f * (vmin + vmax);
	XMVECTOR E = 0.5f * (vmax - vmin);

	XMStoreFloat3(&bounds.Center, C);
	XMStoreFloat3(&bounds.Extents, E);

	return bounds;
}

void Octree::BuildOctree(OctreeNode* parent, const std::vector<DWORD>& indices)
{
	size_t triCount = indices.size() / 3;

	if (triCount < 60)
	{
		parent->IsLeaf = true;
		parent->Indices = indices;
	}
	else
	{
		parent->IsLeaf = false;

		BoundingBox subbox[8];
		parent->Subdivide(subbox);

		for (int i = 0; i < 8; ++i)
		{
			parent->Children[i] = new OctreeNode();
			parent->Children[i]->Bounds = subbox[i];

			std::vector<DWORD> intersectedTriangleIndices;

			for (size_t j = 0; j < triCount; ++j)
			{
				UINT i0 = indices[j * 3 + 0];
				UINT i1 = indices[j * 3 + 1];
				UINT i2 = indices[j * 3 + 2];

				XMVECTOR v0 = XMLoadFloat3(&mVertices[i0]);
				XMVECTOR v1 = XMLoadFloat3(&mVertices[i1]);
				XMVECTOR v2 = XMLoadFloat3(&mVertices[i2]);

				if (subbox[i].Intersects(v0, v1, v2))
				{
					intersectedTriangleIndices.push_back(i0);
					intersectedTriangleIndices.push_back(i1);
					intersectedTriangleIndices.push_back(i2);
				}
			}

			BuildOctree(parent->Children[i], intersectedTriangleIndices);
		}
	}
}

bool Octree::RayOctreeIntersect(OctreeNode* parent, FXMVECTOR rayPos, FXMVECTOR rayDir)
{
	if (!parent->IsLeaf)
	{
		for (int i = 0; i < 8; ++i)
		{
			float t;
			if (parent->Children[i]->Bounds.Intersects(rayPos, rayDir, t))
			{
				if(RayOctreeIntersect(parent->Children[i], rayPos, rayDir))
				{
					return	true;
				}
			}
		}

		return false;
	}
	else
	{
		size_t triCount = parent->Indices.size() / 3;

		for (size_t i = 0; i < triCount; ++i)
		{
			UINT i0 = parent->Indices[i * 3 + 0];
			UINT i1 = parent->Indices[i * 3 + 1];
			UINT i2 = parent->Indices[i * 3 + 2];

			XMVECTOR v0 = XMLoadFloat3(&mVertices[i0]);
			XMVECTOR v1 = XMLoadFloat3(&mVertices[i1]);
			XMVECTOR v2 = XMLoadFloat3(&mVertices[i2]);

			float t;
			if (TriangleTests::Intersects(rayPos, rayDir, v0, v1, v2, t))
			{
				return	true;
			}
		}

		return false;
	}
}
