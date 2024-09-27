#include "Mesh.h"

Mesh::Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex::Vertex>& vertices,
	std::vector<DWORD>& indices, std::vector<Texture>& textures)
{
	this->deviceContext = deviceContext;
	this->textures = textures;
	this->mSkinnedData = new SkinnedData();


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
	this->mSkinnedData = mesh.mSkinnedData;
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

void Mesh::SetSkinnedData(std::vector<BoneInfo>& boneInfos, std::map<std::string, AnimationClip>& map, const aiScene* scene)
{
	mSkinnedData->Set(boneInfos, map);
	mSkinnedData->pScene = scene;

	auto inverseRoot = scene->mRootNode->mTransformation.Inverse().Transpose();
	XMFLOAT4X4 inverMat;

	LoadMatrix(inverMat, inverseRoot);
	mSkinnedData->m_GlobalInverseTransform = XMLoadFloat4x4(&inverMat);
}

void Mesh::LoadMatrix(XMFLOAT4X4& matrixOut, const aiMatrix4x4& matrixIn)
{
	matrixOut(0, 0) = matrixIn.a1;
	matrixOut(0, 1) = matrixIn.a2;
	matrixOut(0, 2) = matrixIn.a3;
	matrixOut(0, 3) = matrixIn.a4;

	matrixOut(1, 0) = matrixIn.b1;
	matrixOut(1, 1) = matrixIn.b2;
	matrixOut(1, 2) = matrixIn.b3;
	matrixOut(1, 3) = matrixIn.b4;

	matrixOut(2, 0) = matrixIn.c1;
	matrixOut(2, 1) = matrixIn.c2;
	matrixOut(2, 2) = matrixIn.c3;
	matrixOut(2, 3) = matrixIn.c4;

	matrixOut(3, 0) = matrixIn.d1;
	matrixOut(3, 1) = matrixIn.d2;
	matrixOut(3, 2) = matrixIn.d3;
	matrixOut(3, 3) = matrixIn.d4;
}