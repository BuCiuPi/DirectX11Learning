#include "Model.h"

#include <map>
#include <sstream>

#include "SkinnedData.h"
#include "StringHelper.h"

bool Model::Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader)
{
	this->device = device;
	this->deviceContext = deviceContext;
	this->cb_vs_vertexshader = cb_vs_vertexshader;

	this->mMaterial.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	this->mMaterial.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	this->mMaterial.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
	this->mMaterial.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	if (!this->LoadModel(filePath))
		return false;

	this->FinalTransform.resize(GetBoneCount());
	return true;
}

void Model::Update(float dt)
{
	TimePos += dt;

	for (int i = 0; i < meshes.size(); ++i)
	{
		meshes[i].mSkinnedData->GetFirstClipFinalTransforms(TimePos, FinalTransform);
	}

	if (TimePos > meshes[0].mSkinnedData->GetFirstClipEndTime())
	{
		TimePos = 0.0f;
	}

	std::wostringstream s;
	s << "\n TIme :" << TimePos;
	OutputDebugString(s.str().c_str());
}

void Model::Draw(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjectionMatrix)
{
	//Update Constant buffer with WVP Matrix
	this->cb_vs_vertexshader->data.gWorld = XMMatrixTranspose(worldMatrix);
	XMMATRIX MVP = worldMatrix * viewProjectionMatrix;
	this->cb_vs_vertexshader->data.gWorldViewProj = XMMatrixTranspose(MVP);
	XMVECTOR detBox = XMMatrixDeterminant(worldMatrix);
	this->cb_vs_vertexshader->data.gWorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(&detBox, worldMatrix));
	this->cb_vs_vertexshader->data.gTexTransform = XMMatrixIdentity();
	this->cb_vs_vertexshader->data.material = mMaterial;
	this->cb_vs_vertexshader->ApplyChanges();

	this->deviceContext->VSSetConstantBuffers(0, 1, this->cb_vs_vertexshader->GetAddressOf());
	this->deviceContext->PSSetConstantBuffers(0, 1, this->cb_vs_vertexshader->GetAddressOf());

	for (int i = 0; i < meshes.size(); i++)
	{
		meshes[i].Draw();
	}
}

bool Model::LoadModel(const std::string& filePath)
{
	this->directory = StringHelper::GetDirectoryFromPath(filePath);
	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(filePath,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded | aiProcess_PopulateArmatureData);

	if (pScene == nullptr)
		return false;

	this->ProcessNode(pScene->mRootNode, pScene);
	return true;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(this->ProcessMesh(mesh, scene));
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		this->ProcessNode(node->mChildren[i], scene);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	// Data to fill
	std::vector<Vertex::Vertex> vertices;
	std::vector<DWORD> indices;

	//Get vertices
	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex::Vertex vertex;

		vertex.pos.x = mesh->mVertices[i].x;
		vertex.pos.y = mesh->mVertices[i].y;
		vertex.pos.z = mesh->mVertices[i].z;

		if (mesh->mNormals != nullptr)
		{
			vertex.normal.x = mesh->mNormals[i].x;
			vertex.normal.y = mesh->mNormals[i].y;
			vertex.normal.z = mesh->mNormals[i].z;
		}

		if (mesh->mTextureCoords[0])
		{
			vertex.texCoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texCoord.y = (float)mesh->mTextureCoords[0][i].y;
		}

		vertex.weights = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertex.BoneIndices[0] = 0;
		vertex.BoneIndices[1] = 0;
		vertex.BoneIndices[2] = 0;
		vertex.BoneIndices[3] = 0;

		vertices.push_back(vertex);
	}

	//Get indices
	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	std::vector<Texture> textures;
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	std::vector<Texture> diffuseTextures = LoadMaterialTextures(material, aiTextureType::aiTextureType_DIFFUSE, scene);
	textures.insert(textures.end(), diffuseTextures.begin(), diffuseTextures.end());

	std::vector<BoneInfo> boneIndexToParentIndex;
	std::map<std::string, AnimationClip> animations;

	ReadBoneHierarchy(mesh, boneIndexToParentIndex);
	ReadAnimationClip(scene, boneIndexToParentIndex.size(), animations, boneIndexToParentIndex);

	ConstructBoneWeightPerVertex(mesh, vertices);

	Mesh curMesh(this->device, this->deviceContext, vertices, indices, textures);
	curMesh.SetSkinnedData(boneIndexToParentIndex, animations);

	return curMesh;
}

void Model::ReadBoneHierarchy(aiMesh* mesh, std::vector<BoneInfo>& BoneIndexToParentIndex)
{
	std::string ignore;
	//BoneIndexToParentIndex.resize(1);

	int currentNodeHierarchy = 0;
	LoadBone(mesh, mesh->mBones[0]->mNode, BoneIndexToParentIndex, -1);
}

void Model::LoadBone(aiMesh* mesh, aiNode* node, std::vector<BoneInfo>& BoneIndexToParentIndex, int currentNodeIndex)
{
	BoneInfo boneInfo;
	boneInfo.HierarchyID = currentNodeIndex;
	boneInfo.BoneName = node->mName.C_Str();

	auto mOffsetMatrix = GetBoneOffsetFromNode(mesh, node, boneInfo);
	auto mTransformation = node->mTransformation;

	LoadMatrix(boneInfo.boneOffset,mOffsetMatrix * mTransformation.Transpose());

	BoneIndexToParentIndex.push_back(boneInfo);

	for (int i = 0; i < node->mNumChildren; ++i)
	{
		LoadBone(mesh, node->mChildren[i], BoneIndexToParentIndex, currentNodeIndex + 1);
	}
}

aiMatrix4x4 Model::GetBoneOffsetFromNode(aiMesh* mesh, aiNode* node, BoneInfo& boneInfo)
{
	for (int i = 0; i < mesh->mNumBones; ++i)
	{
		if (mesh->mBones[i]->mName == node->mName)
		{
			auto mOffsetMatrix = mesh->mBones[i]->mOffsetMatrix.Transpose();
			return mOffsetMatrix;
		}
	}

	XMStoreFloat4x4(&boneInfo.boneOffset, XMMatrixIdentity());
}

void Model::LoadMatrix(XMFLOAT4X4& matrixOut, const aiMatrix4x4& matrixIn)
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

void Model::ConstructBoneWeightPerVertex(const aiMesh* mesh, std::vector<Vertex::Vertex>& vertices)
{
	for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
	{
		for (int i = 0; i < mesh->mBones[boneIndex]->mNumWeights; ++i)
		{
			auto vertexWeight = mesh->mBones[boneIndex]->mWeights[i];
			Vertex::Vertex* currentVertex = &vertices[vertexWeight.mVertexId];

			std::wostringstream s;
			s << "\nBoneIndex: " << boneIndex << "- Weight: " << vertexWeight.mWeight << " - VertexID: " << vertexWeight.mVertexId;
			OutputDebugString(s.str().c_str());

			if (currentVertex->weights.z < vertexWeight.mWeight)
			{
				currentVertex->weights.z = vertexWeight.mWeight;
				currentVertex->BoneIndices[2] = (BYTE)boneIndex;
				continue;
			}

			if (currentVertex->weights.y < vertexWeight.mWeight)
			{
				currentVertex->weights.y = vertexWeight.mWeight;
				currentVertex->BoneIndices[1] = (BYTE)boneIndex;
				continue;
			}

			if (currentVertex->weights.x < vertexWeight.mWeight)
			{
				currentVertex->weights.x = vertexWeight.mWeight;
				currentVertex->BoneIndices[0] = (BYTE)boneIndex;
				continue;
			}
		}
	}
}

void Model::ReadAnimationClip(const aiScene* scene, int numBones, std::map<std::string, AnimationClip>& animations, const std::vector<BoneInfo> boneInfos)
{
	for (int clipIndex = 0; clipIndex < scene->mNumAnimations; ++clipIndex)
	{
		aiAnimation* aiAnimation = scene->mAnimations[clipIndex];
		std::string clipName = aiAnimation->mName.C_Str();

		AnimationClip clip;
		clip.BoneAnimations.resize(numBones);

		for (int boneIndex = 0; boneIndex < numBones; ++boneIndex)
		{
			BoneInfo boneInfo = boneInfos[boneIndex];

			for (int channelIndex = 0; channelIndex < aiAnimation->mNumChannels; ++channelIndex)
			{
				aiNodeAnim* aiNodeAnim = aiAnimation->mChannels[channelIndex];
				std::string chanelName = aiNodeAnim->mNodeName.C_Str();

				int boneIndex = GetBoneIndexByName(chanelName, boneInfos);
				if (boneIndex < 0)
				{
					continue;
				}

				clip.BoneAnimations[boneIndex].keyFrames.resize(aiNodeAnim->mNumPositionKeys);
				for (int keyframeIndex = 0; keyframeIndex < aiNodeAnim->mNumPositionKeys; ++keyframeIndex)
				{
					aiVectorKey position = aiNodeAnim->mPositionKeys[keyframeIndex];
					clip.BoneAnimations[boneIndex].keyFrames[keyframeIndex].Translation = XMFLOAT3(position.mValue.x, position.mValue.y, position.mValue.z);
					clip.BoneAnimations[boneIndex].keyFrames[keyframeIndex].TimePos = position.mTime;

					aiQuatKey rotationQuat = aiNodeAnim->mRotationKeys[keyframeIndex];
					clip.BoneAnimations[boneIndex].keyFrames[keyframeIndex].RotationQuat = XMFLOAT4(rotationQuat.mValue.x, rotationQuat.mValue.y, rotationQuat.mValue.z, rotationQuat.mValue.w);

					aiVectorKey scale = aiNodeAnim->mScalingKeys[keyframeIndex];
					clip.BoneAnimations[boneIndex].keyFrames[keyframeIndex].Translation = XMFLOAT3(scale.mValue.x, scale.mValue.y, scale.mValue.z);
				}
			}
		}

		animations[clipName] = clip;
	}
}

int Model::GetBoneIndexByName(std::string name, const std::vector<BoneInfo>& boneInfos)
{
	for (int i = 0; i < boneInfos.size(); ++i)
	{
		if (boneInfos[i].BoneName == name)
		{
			return i;
		}
	}

	return -1;
}

int Model::GetBoneCount()
{
	return this->meshes[0].mSkinnedData->BoneCount();
}

TextureStorageType Model::DetermineTextureStorageType(const aiScene* pScene, aiMaterial* pMat, unsigned int index, aiTextureType textureType)
{
	if (pMat->GetTextureCount(textureType) == 0)
		return TextureStorageType::None;

	aiString path;
	pMat->GetTexture(textureType, index, &path);
	std::string texturePath = path.C_Str();
	//Check if texture is an embedded indexed texture by seeing if the file path is an index #
	if (texturePath[0] == '*')
	{
		if (pScene->mTextures[0]->mHeight == 0)
		{
			return TextureStorageType::EmbeddedIndexCompressed;
		}
		else
		{
			assert("SUPPORT DOES NOT EXIST YET FOR INDEXED NON COMPRESSED TEXTURES!" && 0);
			return TextureStorageType::EmbeddedIndexNonCompressed;
		}
	}
	//Check if texture is an embedded texture but not indexed (path will be the texture's name instead of #)
	if (auto pTex = pScene->GetEmbeddedTexture(texturePath.c_str()))
	{
		if (pTex->mHeight == 0)
		{
			return TextureStorageType::EmbeddedCompressed;
		}
		else
		{
			assert("SUPPORT DOES NOT EXIST YET FOR EMBEDDED NON COMPRESSED TEXTURES!" && 0);
			return TextureStorageType::EmbeddedNonCompressed;
		}
	}
	//Lastly check if texture is a filepath by checking for period before extension name
	if (texturePath.find('.') != std::string::npos)
	{
		return TextureStorageType::Disk;
	}

	return TextureStorageType::None; // No texture exists
}

std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene)
{
	std::vector<Texture> materialTextures;
	TextureStorageType storetype = TextureStorageType::Invalid;
	unsigned int textureCount = pMaterial->GetTextureCount(textureType);

	if (textureCount == 0) //If there are no textures
	{
		storetype = TextureStorageType::None;
		aiColor3D aiColor(0.0f, 0.0f, 0.0f);
		switch (textureType)
		{
		case aiTextureType_DIFFUSE:
			pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
			if (aiColor.IsBlack()) //If color = black, just use grey
			{
				materialTextures.push_back(Texture(this->device, ColorHelpers::UnloadedTextureColor, textureType));
				return materialTextures;
			}
			materialTextures.push_back(Texture(this->device, ColorHelper(aiColor.r * 255, aiColor.g * 255, aiColor.b * 255), textureType));
			return materialTextures;
		}
	}
	else
	{
		for (UINT i = 0; i < textureCount; i++)
		{
			aiString path;
			pMaterial->GetTexture(textureType, i, &path);
			TextureStorageType storetype = DetermineTextureStorageType(pScene, pMaterial, i, textureType);
			switch (storetype)
			{
			case TextureStorageType::Disk:
			{
				std::string filename = this->directory + '\\' + path.C_Str();
				Texture diskTexture(this->device, filename, textureType);
				materialTextures.push_back(diskTexture);
				break;
			}
			}
		}
	}

	if (materialTextures.size() == 0)
	{
		materialTextures.push_back(Texture(this->device, ColorHelpers::UnhandledTextureColor, aiTextureType::aiTextureType_DIFFUSE));
	}
	return materialTextures;
}