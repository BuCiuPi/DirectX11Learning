#include "Model.h"

#include <map>
#include <sstream>

#include "SkinnedData.h"
#include "StringHelper.h"

bool Model::Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader, ConstantBuffer<CB_VS_Skinned>* cb_vs_skinnedBuffer)
{
	this->device = device;
	this->deviceContext = deviceContext;
	this->cb_vs_vertexshader = cb_vs_vertexshader;
	this->cb_vs_skinnedBuffer = cb_vs_skinnedBuffer;


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
	TimePos += 20 * dt;

	for (int i = 0; i < meshes.size(); ++i)
	{
		meshes[i].mSkinnedData->GetBoneTransforms(TimePos);
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

	for (int meshIndex = 0; meshIndex < meshes.size(); meshIndex++)
	{
		for (int i = 0; i < meshes[meshIndex].mSkinnedData->mBoneHierarchy.size(); ++i)
		{
			cb_vs_skinnedBuffer->data.gBoneTransform[i] = (meshes[meshIndex].mSkinnedData->mBoneHierarchy[i].FinalTransform);
			//BoneInfo* boneInfo = &meshes[meshIndex].mSkinnedData->mBoneHierarchy[i];
			//XMMATRIX finalTransform = boneInfo->boneOffset * boneInfo->globalTransform;
			//cb_vs_skinnedBuffer->data.gBoneTransform[i] = finalTransform;
			//cb_vs_skinnedBuffer->data.gBoneTransform[i] = XMMatrixIdentity();
		}
		cb_vs_skinnedBuffer->ApplyChanges();
		cb_vs_skinnedBuffer->VSShaderUpdate(2);

		meshes[meshIndex].Draw();
	}
}

bool Model::LoadModel(const std::string& filePath)
{
	this->directory = StringHelper::GetDirectoryFromPath(filePath);
	Assimp::Importer importer;

	pScene = importer.ReadFile(filePath,
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

	ReadBoneHierarchy(mesh, boneIndexToParentIndex, vertices);
	ReadAnimationClip(scene, boneIndexToParentIndex.size(), animations, boneIndexToParentIndex);

	//ConstructBoneWeightPerVertex(mesh, vertices);

	Mesh curMesh(this->device, this->deviceContext, vertices, indices, textures);
	curMesh.SetSkinnedData(boneIndexToParentIndex, animations, scene);

	return curMesh;
}

void Model::ReadBoneHierarchy(aiMesh* mesh, std::vector<BoneInfo>& BoneIndexToParentIndex, std::vector<Vertex::Vertex>& vertices)
{

	for (int i = 0; i < mesh->mNumBones; ++i)
	{
		LoadBone(mesh, mesh->mBones[i], i, BoneIndexToParentIndex, vertices);
	}

	for (int i = 0; i < BoneIndexToParentIndex.size(); ++i)
	{
		BoneInfo* curBoneInfo = &BoneIndexToParentIndex[i];
		if (curBoneInfo->HierarchyID != -1)
		{
			curBoneInfo->Parent = &BoneIndexToParentIndex[curBoneInfo->HierarchyID];
		}

		// get globalTransform
		BoneInfo* curParent = curBoneInfo->Parent;
		curBoneInfo->globalTransform = curBoneInfo->localTransform;
		while (curParent != nullptr)
		{
			curBoneInfo->globalTransform *= curParent->localTransform;
			curParent = curParent->Parent;
		}
		XMFLOAT4X4 globalTransform;
		XMStoreFloat4x4(&globalTransform, curBoneInfo->globalTransform);
		curBoneInfo->originalGlobalTransform = XMLoadFloat4x4(&globalTransform);

		for (int j = i; j < BoneIndexToParentIndex.size(); ++j)
		{
			if (BoneIndexToParentIndex[j].HierarchyID == i)
			{
				curBoneInfo->Childrents.push_back(BoneIndexToParentIndex[j]);
			}
		}
	}
}

void Model::LoadBone(aiMesh* mesh, aiBone* bone, int boneIndex, std::vector<BoneInfo>& BoneIndexToParentIndex, std::vector<Vertex::Vertex>& vertices)
{
	BoneInfo boneInfo;
	boneInfo.BoneName = bone->mName.C_Str();
	boneInfo.HierarchyID = -1;

	if (bone->mNode->mParent != bone->mArmature)
	{
		for (int i = 0; i < BoneIndexToParentIndex.size(); ++i)
		{

			if (BoneIndexToParentIndex[i].BoneName == bone->mNode->mParent->mName.C_Str())
			{
				boneInfo.HierarchyID = i;
				break;
			}

			if (boneInfo.HierarchyID != -1)
			{
				break;
			}
		}
	}

	auto mOffsetMatrix = bone->mOffsetMatrix;
	auto mTransformation = bone->mNode->mTransformation;

	LoadMatrix(boneInfo.boneOffset, mOffsetMatrix.Transpose());
	LoadMatrix(boneInfo.localTransform, mTransformation.Transpose());
	boneInfo.originalLocalTransform = boneInfo.localTransform;

	for (int i = 0; i < bone->mNumWeights; ++i)
	{
		auto vertexWeight = bone->mWeights[i];
		Vertex::Vertex* currentVertex = &vertices[vertexWeight.mVertexId];

		std::wostringstream s;
		s << "\nBoneIndex: " << boneIndex << "- Weight: " << vertexWeight.mWeight << " - VertexID: " << vertexWeight.mVertexId;
		OutputDebugString(s.str().c_str());

		if (currentVertex->weights.x == 0.0f)
		{
			currentVertex->weights.x = vertexWeight.mWeight;
			currentVertex->BoneIndices[0] = (BYTE)boneIndex;
			continue;
		}

		if (currentVertex->weights.y == 0.0f)
		{
			currentVertex->weights.y = vertexWeight.mWeight;
			currentVertex->BoneIndices[1] = (BYTE)boneIndex;
			continue;
		}

		if (currentVertex->weights.z == 0.0f)
		{
			currentVertex->weights.z = vertexWeight.mWeight;
			currentVertex->BoneIndices[2] = (BYTE)boneIndex;
			continue;
		}
	}

	BoneIndexToParentIndex.push_back(boneInfo);
}

void Model::CalculatedBOneToWorldTransform(BoneInfo boneInfo)
{

}

void Model::LoadMatrix(XMMATRIX& matrixOut, const aiMatrix4x4& matrixIn)
{
	XMFLOAT4X4 loadedMatrix;
	loadedMatrix(0, 0) = matrixIn.a1;
	loadedMatrix(0, 1) = matrixIn.a2;
	loadedMatrix(0, 2) = matrixIn.a3;
	loadedMatrix(0, 3) = matrixIn.a4;

	loadedMatrix(1, 0) = matrixIn.b1;
	loadedMatrix(1, 1) = matrixIn.b2;
	loadedMatrix(1, 2) = matrixIn.b3;
	loadedMatrix(1, 3) = matrixIn.b4;

	loadedMatrix(2, 0) = matrixIn.c1;
	loadedMatrix(2, 1) = matrixIn.c2;
	loadedMatrix(2, 2) = matrixIn.c3;
	loadedMatrix(2, 3) = matrixIn.c4;

	loadedMatrix(3, 0) = matrixIn.d1;
	loadedMatrix(3, 1) = matrixIn.d2;
	loadedMatrix(3, 2) = matrixIn.d3;
	loadedMatrix(3, 3) = matrixIn.d4;

	//loadedMatrix(0, 0) = matrixIn.a1;
	//loadedMatrix(1, 0) = matrixIn.a2;
	//loadedMatrix(2, 0) = matrixIn.a3;
	//loadedMatrix(3, 0) = matrixIn.a4;

	//loadedMatrix(0, 1) = matrixIn.b1;
	//loadedMatrix(1, 1) = matrixIn.b2;
	//loadedMatrix(2, 1) = matrixIn.b3;
	//loadedMatrix(3, 1) = matrixIn.b4;

	//loadedMatrix(0, 2) = matrixIn.c1;
	//loadedMatrix(1, 2) = matrixIn.c2;
	//loadedMatrix(2, 2) = matrixIn.c3;
	//loadedMatrix(3, 2) = matrixIn.c4;

	//loadedMatrix(0, 3) = matrixIn.d1;
	//loadedMatrix(1, 3) = matrixIn.d2;
	//loadedMatrix(2, 3) = matrixIn.d3;
	//loadedMatrix(3, 3) = matrixIn.d4;

	matrixOut = XMLoadFloat4x4(&loadedMatrix);
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
				//currentVertex->weights.z = vertexWeight.mWeight;
				currentVertex->BoneIndices[2] = (BYTE)boneIndex;
				continue;
			}

			if (currentVertex->weights.y < vertexWeight.mWeight)
			{
				//currentVertex->weights.y = vertexWeight.mWeight;
				currentVertex->BoneIndices[1] = (BYTE)boneIndex;
				continue;
			}

			if (currentVertex->weights.x < vertexWeight.mWeight)
			{
				//currentVertex->weights.x = vertexWeight.mWeight;
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
			std::string boneName = boneInfo.BoneName;

			aiNodeAnim* aiNodeAnim = aiAnimation->mChannels[boneIndex];
			std::string chanelName = aiNodeAnim->mNodeName.C_Str();
			clip.BoneAnimations[boneIndex].Name = chanelName;

			//int boneIndex = GetBoneIndexByName(chanelName, boneInfos);
			//if (boneIndex < 0)
			//{
			//	continue;
			//}


			clip.BoneAnimations[boneIndex].keyFrames.resize(aiNodeAnim->mNumPositionKeys);
			for (int keyframeIndex = 0; keyframeIndex < aiNodeAnim->mNumPositionKeys; ++keyframeIndex)
			{
				aiVectorKey position = aiNodeAnim->mPositionKeys[keyframeIndex];
				clip.BoneAnimations[boneIndex].keyFrames[keyframeIndex].Translation = XMFLOAT3(position.mValue.x, position.mValue.y, position.mValue.z);
				clip.BoneAnimations[boneIndex].keyFrames[keyframeIndex].TimePos = position.mTime;

				aiQuatKey rotationQuat = aiNodeAnim->mRotationKeys[keyframeIndex];
				clip.BoneAnimations[boneIndex].keyFrames[keyframeIndex].RotationQuat = XMFLOAT4(rotationQuat.mValue.x, rotationQuat.mValue.y, rotationQuat.mValue.z, rotationQuat.mValue.w);

				aiVectorKey scale = aiNodeAnim->mScalingKeys[keyframeIndex];
				clip.BoneAnimations[boneIndex].keyFrames[keyframeIndex].Scale = XMFLOAT3(scale.mValue.x, scale.mValue.y, scale.mValue.z);
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