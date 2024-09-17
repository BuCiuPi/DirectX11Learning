#include "Mesh.h"

#include <sstream>

#include "Octree.h"

Mesh::Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex::Vertex>& vertices,
           std::vector<DWORD>& indices, std::vector<Texture>& textures)
{
	this->deviceContext = deviceContext;
	this->textures = textures;

	BuildVertexAmbientOcclusion(vertices, indices);

	HRESULT hr = this->vertexbuffer.Initialize(device, vertices.data(), vertices.size());
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"Failed to initialize vertex buffer for mesh.", L"Error", hr);
	}

	hr = this->indexbuffer.Initialize(device, indices.data(), indices.size());
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"Failed to initialize index buffer for mesh.", L"Error", hr);
	}
}

Mesh::Mesh(const Mesh& mesh)
{
	this->deviceContext = mesh.deviceContext;
	this->indexbuffer = mesh.indexbuffer;
	this->vertexbuffer = mesh.vertexbuffer;
	this->textures = mesh.textures;
}

void Mesh::Draw()
{
	UINT offset = 0;

	for (int i = 0; i < textures.size(); i++)
	{
		if (textures[i].GetType() == aiTextureType::aiTextureType_DIFFUSE)
		{
			this->deviceContext->PSSetShaderResources(0, 1, textures[i].GetTextureResourceViewAddress());
			break;
		}
	}

	this->deviceContext->IASetVertexBuffers(0, 1, this->vertexbuffer.GetAddressOf(), this->vertexbuffer.StridePtr(), &offset);
	this->deviceContext->IASetIndexBuffer(this->indexbuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(this->indexbuffer.BufferSize(), 0, 0);
}

void Mesh::BuildVertexAmbientOcclusion(std::vector<Vertex::Vertex>& vertices, const std::vector<DWORD> indices)
{
	UINT vcount = vertices.size();
	UINT tcount = indices.size() / 3;

	std::vector<XMFLOAT3> positions(vcount);

	for (int i = 0; i < vcount; ++i)
	{
		positions[i] = vertices[i].pos;
		vertices[i].AmbientAccess = 0;
	}

	Octree octree;
	octree.Build(positions, indices);

	std::vector<int> vertexSharedCount(vcount);

	for (int i = 0; i < tcount; ++i)
	{
		UINT i0 = indices[i * 3 + 0];
		UINT i1 = indices[i * 3 + 1];
		UINT i2 = indices[i * 3 + 2];

		XMVECTOR v0 = XMLoadFloat3(&vertices[i0].pos);
		XMVECTOR v1 = XMLoadFloat3(&vertices[i1].pos);
		XMVECTOR v2 = XMLoadFloat3(&vertices[i2].pos);

		XMVECTOR edge0 = v1 - v0;
		XMVECTOR edge1 = v2 - v0;

		XMVECTOR normal = XMVector3Normalize(XMVector3Cross(edge0, edge1));

		XMVECTOR centroid = (v0 + v1 + v2) / 3.0f;

		centroid += 0.01f * normal;

		const int numSampleRays = 32;
		float numUnoccluded = 0;

		for (int i = 0; i < numSampleRays; ++i)
		{
			XMVECTOR randomDir = MathHelper::RandHemisphereUnitVec3(normal);

			if (!octree.RayOctreeIntersect(centroid, randomDir))
			{
				numUnoccluded++;
			}
		}

		float ambientAccess = numUnoccluded / numSampleRays;

		vertices[i0].AmbientAccess += ambientAccess;
		vertices[i1].AmbientAccess += ambientAccess;
		vertices[i2].AmbientAccess += ambientAccess;

		vertexSharedCount[i0]++;
		vertexSharedCount[i1]++;
		vertexSharedCount[i2]++;

		std::wostringstream ss;
		ss << L"Build Tris: " << ambientAccess;

		OutputDebugString(reinterpret_cast<LPCWSTR>(ss.str().c_str()));
	}

	for (int i = 0; i < vcount; ++i)
	{
		vertices[i].AmbientAccess /= vertexSharedCount[i];
	}
}

