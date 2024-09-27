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

#include "SkinnedData.h"
#include "Texture.h"

class Mesh
{
public:
	Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex::Vertex>& vertices,
	     std::vector<DWORD>& indices, std::vector<Texture>& textures);
	Mesh(const Mesh& mesh);

	void Update(float dt);
	void Draw();
	void SetSkinnedData(std::vector<BoneInfo>& boneInfos, std::map<std::string, AnimationClip>& map, const aiScene* scene);
	void LoadMatrix(XMFLOAT4X4& matrixOut, const aiMatrix4x4& matrixIn);

	SkinnedData* mSkinnedData;
private:
	VertexBuffer<Vertex::Vertex> vertexbuffer;
	IndexBuffer indexbuffer;
	ID3D11DeviceContext* deviceContext;
	std::vector<Texture> textures;
};

#endif


