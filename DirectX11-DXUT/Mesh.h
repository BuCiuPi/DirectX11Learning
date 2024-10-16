#ifndef MESH_H
#define MESH_H
#include <d3d11.h>
#include <vector>
#include "D3DUtil.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "ModelTexture.h"

class Mesh
{
public:
	Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex::Vertex>& vertices,
	     std::vector<DWORD>& indices, std::vector<ModelTexture>& textures);
	Mesh(const Mesh& mesh);

	void Draw();

private:
	VertexBuffer<Vertex::Vertex> vertexbuffer;
	IndexBuffer indexbuffer;
	ID3D11DeviceContext* deviceContext;
	std::vector<ModelTexture> textures;
};

#endif


