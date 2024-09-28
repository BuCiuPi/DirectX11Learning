#include "Mesh.h"

Mesh::Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex::Vertex>& vertices,
	std::vector<DWORD>& indices, std::vector<Texture>& textures)
{
	this->deviceContext = deviceContext;
	this->textures = textures;

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