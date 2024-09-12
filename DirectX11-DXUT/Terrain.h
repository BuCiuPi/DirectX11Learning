#ifndef TERRAIN_H
#define TERRAIN_H

#include "D3DUtil.h"

class Terrain
{
public:
	struct InitInfo
	{
		std::wstring HeightMapFileName;
		std::wstring BlendMapFileName;
		LPCTSTR LayerFileNames[5];
		float HeightScale;
		UINT HeightmapWidth;
		UINT HeightmapHeight;
		float CellSpacing;
	};

	Terrain();
	~Terrain();

	float GetWidth() const;
	float GetDepth() const;
	float GetHeight(int x, int z) const;

	XMMATRIX GetWorld() const;

	void SetWorld(CXMMATRIX M);

	void Init(ID3D11Device* device, ID3D11DeviceContext* deviceContex, const InitInfo& initInfo);
	void SetConstantBuffer(ID3D11DeviceContext* deviceContext, const Camera& mCamera, DirectionalLight mDirLights[3]);
	void Draw(ID3D11DeviceContext* deviceContext, const Camera& mCamera, DirectionalLight mDirLights[3]);

	void BuildTerrainFX(ID3D11Device* device);

	InitInfo mInfo;
	Material mMat;
private:
	void LoadHeightMap();
	void Smooth();
	bool InBounds(int i, int j);
	float Average(int i, int j);

	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(UINT i, UINT j);
	void BuildQuadPatchVB(ID3D11Device* device);
	void BuildQuadPatchIB(ID3D11Device* device);
	void BuildHeightMapSRV(ID3D11Device* device);

private:
	static const int CellsPerPatch = 64;

	ID3D11Buffer* mTerrainConstantBuffer = nullptr;
	ID3D11Buffer* mTerrainPerFrameBuffer = nullptr;

	ID3D11VertexShader* mTerrainVertexShader = nullptr;
	ID3D11HullShader* mTerrainHullShader = nullptr;
	ID3D11DomainShader* mTerrainDomainShader = nullptr;
	ID3D11PixelShader* mTerrainPixelShader = nullptr;


	ID3D11Buffer* mQuadPatchVB = nullptr;
	ID3D11Buffer* mQuadPatchIB = nullptr;

	ID3D11ShaderResourceView* mLayerMapArraySRV = nullptr;
	ID3D11ShaderResourceView* mBlendMapSRV = nullptr;
	ID3D11ShaderResourceView* mHeightMapSRV = nullptr;

	UINT mNumPatchVertices;
	UINT mNumPatchQuadFaces;

	UINT mNumPatchVertRows;
	UINT mNumPatchVertCols;

	XMFLOAT4X4 mWorld;
	ID3D11Texture2D* mTerrainTexture = nullptr;

	ID3D11SamplerState* mSamLinear = nullptr;
	ID3D11SamplerState* mSamHeightMap = nullptr;

	std::vector<XMFLOAT2> mPatchBoundsY;
	std::vector<float> mHeightMap;

};

#endif


