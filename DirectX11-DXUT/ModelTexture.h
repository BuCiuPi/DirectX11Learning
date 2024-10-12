#ifndef MODEL_TEXTURE_H
#define MODEL_TEXTURE_H
#include <d3d11.h>
#include <assimp/material.h>
#include <wrl/client.h>
#include "ColorHelper.h"

enum class TextureStorageType
{
	Invalid,
	None,
	EmbeddedIndexCompressed,
	EmbeddedIndexNonCompressed,
	EmbeddedCompressed,
	EmbeddedNonCompressed,
	Disk
};

class ModelTexture
{
public:
	ModelTexture(ID3D11Device* device, const ColorHelper& color, aiTextureType type);
	ModelTexture(ID3D11Device* device, const ColorHelper* colorData, UINT width, UINT height, aiTextureType type); //Generate texture of specific color data
	ModelTexture(ID3D11Device* device, const std::string& filePath, aiTextureType type);

	aiTextureType GetType();
	ID3D11ShaderResourceView* GetTextureResourceView();
	ID3D11ShaderResourceView** GetTextureResourceViewAddress();

private:
	void Initialize1x1ColorTexture(ID3D11Device* device, const ColorHelper& colorData, aiTextureType type);
	void InitializeColorTexture(ID3D11Device* device, const ColorHelper* colorData, UINT width, UINT height, aiTextureType type);
	Microsoft::WRL::ComPtr<ID3D11Resource> texture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView = nullptr;
	aiTextureType type = aiTextureType::aiTextureType_UNKNOWN;
};
#endif


