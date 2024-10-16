#ifndef MODEL_H
#define MODEL_H
#include <d3d11.h>
#include <string>
#include "Mesh.h"


class Model
{
public:
	bool Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void Draw(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjectionMatrix);
private:
	std::vector<Mesh> meshes;
	bool LoadModel(const std::string& filePath);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	TextureStorageType DetermineTextureStorageType(const aiScene* pScene, aiMaterial* pMat, unsigned int index, aiTextureType textureType);
	std::vector<ModelTexture> LoadMaterialTextures(aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene);

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	std::string directory = "";

};
#endif
