#ifndef MODEL_H
#define MODEL_H
#include <d3d11.h>
#include <string>
#include "Mesh.h"
#include "SkinnedData.h"


class Model
{
public:
	bool Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader);
	void Update(float dt);
	void Draw(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjectionMatrix);

	int GetBoneCount();
	std::vector<XMFLOAT4X4> FinalTransform;
private:
	std::vector<Mesh> meshes;
	bool LoadModel(const std::string& filePath);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	void ConstructBoneWeightPerVertex(const aiMesh* mesh, std::vector<Vertex::Vertex>& vertices);
	void ReadBoneHierarchy(aiMesh* mesh, std::vector<BoneInfo>& BoneIndexToParentIndex);
	void LoadBone(aiMesh* mesh, aiNode* node, std::vector<BoneInfo>& BoneIndexToParentIndex, int currentNodeIndex);
	aiMatrix4x4 GetBoneOffsetFromNode(aiMesh* mesh, aiNode* node, BoneInfo& boneInfo);
	void LoadMatrix(XMFLOAT4X4& matrixOut, const aiMatrix4x4& matrixIn);
	void ReadAnimationClip(const aiScene* scene, int numBones, std::map<std::string, AnimationClip>& animations, std::vector<BoneInfo> boneInfos);
	int GetBoneIndexByName(std::string name, const std::vector<BoneInfo>& boneInfos);

	TextureStorageType DetermineTextureStorageType(const aiScene* pScene, aiMaterial* pMat, unsigned int index, aiTextureType textureType);
	std::vector<Texture> LoadMaterialTextures(aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene);

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader = nullptr;
	std::string directory = "";

	Material mMaterial;

	float TimePos;
};
#endif
