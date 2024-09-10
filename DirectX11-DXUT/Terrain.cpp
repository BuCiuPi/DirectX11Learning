#include "Terrain.h"

#include <fstream>

#include "BitmapBuilder.h"

Terrain::Terrain()
{
	mNumPatchVertices = 0;
	mNumPatchQuadFaces = 0;
	mNumPatchVertCols = 0;
	mNumPatchVertRows = 0;

	XMStoreFloat4x4(&mWorld, XMMatrixIdentity());


	mMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
	mMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

Terrain::~Terrain()
{
	ReleaseCOM(mQuadPatchVB);
	ReleaseCOM(mQuadPatchIB);
	ReleaseCOM(mLayerMapArraySRV);
	ReleaseCOM(mBlendMapSRV);
	ReleaseCOM(mHeightMapSRV)
}

float Terrain::GetWidth() const
{
	return (mInfo.HeightmapWidth - 1) * mInfo.CellSpacing;
}

float Terrain::GetDepth() const
{
	return (mInfo.HeightmapHeight - 1) * mInfo.CellSpacing;
}

float Terrain::GetHeight(int x, int z) const
{
	float c = (x + 0.5f * GetWidth()) / mInfo.CellSpacing;
	float d = (z - 0.5f * GetDepth()) / -mInfo.CellSpacing;

	int row = (int)floorf(d);
	int col = (int)floorf(c);

	float A = mHeightMap[row * mInfo.HeightmapWidth + col];
	float B = mHeightMap[row * mInfo.HeightmapWidth + (col + 1)];
	float C = mHeightMap[(row + 1) * mInfo.HeightmapWidth + col];
	float D = mHeightMap[(row + 1) * mInfo.HeightmapWidth + (col + 1)];

	float s = c - (float)col;
	float t = d - (float)row;

	if (s + t <= 1.0f)
	{
		float uy = B - A;
		float vy = C - A;
		return A + s * uy + t * vy;
	}
	else
	{

		float uy = C - D;
		float vy = B - D;
		return D + (1.0f - s) * uy + (1.0f - t) * vy;
	}

}

XMMATRIX Terrain::GetWorld() const
{
	return XMLoadFloat4x4(&mWorld);
}

void Terrain::SetWorld(CXMMATRIX M)
{
	XMStoreFloat4x4(&mWorld, M);
}

void Terrain::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContex, const InitInfo& initInfo)
{
	mInfo = initInfo;

	mNumPatchVertRows = ((mInfo.HeightmapHeight - 1) / CellsPerPatch) + 1;
	mNumPatchVertCols = ((mInfo.HeightmapWidth - 1) / CellsPerPatch) + 1;

	mNumPatchVertices = mNumPatchVertRows * mNumPatchVertCols;
	mNumPatchQuadFaces = (mNumPatchVertRows - 1) * (mNumPatchVertCols - 1);

	LoadHeightMap();
	Smooth();
	CalcAllPatchBoundsY();

	BuildQuadPatchVB(device);
	BuildQuadPatchIB(device);
	BuildHeightMapSRV(device);

	HR(LoadTextureArray(deviceContex, device, mInfo.LayerFileNames, sizeof(mInfo.LayerFileNames) / sizeof(mInfo.LayerFileNames[0]), &mTerrainTexture, &mLayerMapArraySRV));

	HR(CreateDDSTextureFromFile(device, mInfo.BlendMapFileName.c_str(), nullptr, &mBlendMapSRV));

	D3D11_SAMPLER_DESC samHeightDesc = {};
	samHeightDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samHeightDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samHeightDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samHeightDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samHeightDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samHeightDesc.MipLODBias = 0.0f;
	samHeightDesc.MaxAnisotropy = 4;
	samHeightDesc.MinLOD = 0;
	samHeightDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(device->CreateSamplerState(&samHeightDesc, &mSamHeightMap));

	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(device->CreateSamplerState(&sampDesc, &mSamLinear));
}

void Terrain::Draw(ID3D11DeviceContext* deviceContext)
{
	UINT stride = sizeof(Vertex::Terrain);
	UINT offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &mQuadPatchVB, &stride, &offset);
	deviceContext->IASetIndexBuffer(mQuadPatchIB, DXGI_FORMAT_R16_UINT, 0);

	deviceContext->VSSetSamplers(1, 1, &mSamHeightMap);
	deviceContext->DSSetSamplers(1, 1, &mSamHeightMap);
	deviceContext->PSSetSamplers(1, 1, &mSamHeightMap);

	deviceContext->PSSetSamplers(0, 1, &mSamLinear);

	deviceContext->PSSetShaderResources(0, 1, &mLayerMapArraySRV);

	deviceContext->PSSetShaderResources(1, 1, &mBlendMapSRV);

	deviceContext->VSSetShaderResources(2, 1, &mHeightMapSRV);
	deviceContext->DSSetShaderResources(2, 1, &mHeightMapSRV);
	deviceContext->PSSetShaderResources(2, 1, &mHeightMapSRV);

	deviceContext->RSSetState(RenderStates::WireframeRS);

	deviceContext->DrawIndexed(mNumPatchQuadFaces * 4, 0, 0);

	deviceContext->RSSetState(0);
	deviceContext->HSSetShader(0, 0, 0);
	deviceContext->DSSetShader(0, 0, 0);
}

void Terrain::LoadHeightMap()
{

	std::vector<unsigned char> in(mInfo.HeightmapWidth * mInfo.HeightmapHeight);
	std::ifstream inFile;

	inFile.open(mInfo.HeightMapFileName.c_str(), std::ios_base::binary);

	if (inFile)
	{
		inFile.read((char*)&in[0], (std::streamsize)in.size());

		inFile.close();
	}

	int heightMapSize = mInfo.HeightmapWidth * mInfo.HeightmapHeight;
	mHeightMap.resize(heightMapSize, 0);
	for (int i = 0; i < heightMapSize; ++i)
	{
		mHeightMap[i] = (in[i] / 255.0f) * mInfo.HeightScale;
		if (mHeightMap[i] < 0)
		{
			OutputDebugStringW(L"lower");
		}
	}


}

void Terrain::Smooth()
{
	std::vector<float> dest(mHeightMap.size());

	for (int i = 0; i < mInfo.HeightmapHeight; ++i)
	{
		for (int j = 0; j < mInfo.HeightmapWidth; ++j)
		{
			dest[i * mInfo.HeightmapWidth + j] = Average(i, j);
		}
	}

	mHeightMap = dest;
}

bool Terrain::InBounds(int i, int j)
{
	return 	i >= 0 && i < (int)mInfo.HeightmapHeight &&
			j >= 0 && j < (int)mInfo.HeightmapWidth;
}

float Terrain::Average(int i, int j)
{
	float avg = 0.0f;
	float num = 0.0f;

	for (int m = i - 1; m <= i + 1; ++m)
	{
		for (int n = j - 1; n <= j + 1; ++n)
		{
			if (InBounds(m, n))
			{
				avg += mHeightMap[m * mInfo.HeightmapWidth + n];
				num += 1.0f;
			}
		}
	}

	return avg / num;
}

void Terrain::CalcAllPatchBoundsY()
{
	mPatchBoundsY.resize(mNumPatchQuadFaces);

	for (int i = 0; i < mNumPatchVertRows - 1; ++i)
	{
		for (int j = 0; j < mNumPatchVertCols - 1; ++j)
		{
			CalcPatchBoundsY(i, j);
		}
	}
}

void Terrain::CalcPatchBoundsY(UINT i, UINT j)
{
	UINT x0 = j * CellsPerPatch;
	UINT x1 = (j + 1) * CellsPerPatch;

	UINT y0 = i * CellsPerPatch;
	UINT y1 = (i + 1) * CellsPerPatch;

	float minY = +MathHelper::Infinity;
	float maxY = -MathHelper::Infinity;
	for (UINT y = y0; y <= y1; ++y)
	{
		for (UINT x = x0; x <= x1; ++x)
		{
			UINT k = y * mInfo.HeightmapWidth + x;
			minY = MathHelper::Min(minY, mHeightMap[k]);
			maxY = MathHelper::Max(maxY, mHeightMap[k]);
		}
	}

	UINT patchID = i * (mNumPatchVertCols - 1) + j;
	mPatchBoundsY[patchID] = XMFLOAT2(minY, maxY);
}

void Terrain::BuildQuadPatchVB(ID3D11Device* device)
{
	std::vector<Vertex::Terrain> patchVertices(mNumPatchVertRows * mNumPatchVertCols);

	float halfWidth = 0.5f * GetWidth();
	float halfDepth = 0.5f * GetDepth();

	float patchWidth = GetWidth() / (mNumPatchVertCols - 1);
	float patchDepth = GetDepth() / (mNumPatchVertRows - 1);

	float du = 1.0f / (mNumPatchVertCols - 1);
	float dv = 1.0f / (mNumPatchVertRows - 1);

	for (int i = 0; i < mNumPatchVertRows; ++i)
	{
		float z = halfDepth - i * patchDepth;
		for (int j = 0; j < mNumPatchVertCols; ++j)
		{
			float x = -halfWidth + j * patchWidth;

			patchVertices[i * mNumPatchVertCols + j].Pos = XMFLOAT3(x, 0.0f, z);

			patchVertices[i * mNumPatchVertCols + j].Tex.x = j * du;
			patchVertices[i * mNumPatchVertCols + j].Tex.y = i * dv;
		}
	}

	for (int i = 0; i < mNumPatchVertRows - 1; ++i)
	{
		for (int j = 0; j < mNumPatchVertCols - 1; ++j)
		{
			UINT patchID = i * (mNumPatchVertCols - 1) + j;
			patchVertices[i * mNumPatchVertCols + j].BoundsY = mPatchBoundsY[patchID];
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Terrain) * patchVertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = &patchVertices[0];

	HR(device->CreateBuffer(&vbd, &vInitData, &mQuadPatchVB));
}

void Terrain::BuildQuadPatchIB(ID3D11Device* device)
{
	std::vector<USHORT> indices(mNumPatchQuadFaces * 4);

	int k = 0;
	for (int i = 0; i < mNumPatchVertRows - 1; ++i)
	{
		for (int j = 0; j < mNumPatchVertCols - 1; ++j)
		{
			indices[k] = i * mNumPatchVertCols + j;
			indices[k + 1] = i * mNumPatchVertCols + (j + 1);

			indices[k + 2] = (i + 1) * mNumPatchVertCols + j;
			indices[k + 3] = (i + 1) * mNumPatchVertCols + (j + 1);

			k += 4;
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = &indices[0];

	HR(device->CreateBuffer(&ibd, &iInitData, &mQuadPatchIB));
}

void Terrain::BuildHeightMapSRV(ID3D11Device* device)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mInfo.HeightmapWidth;
	texDesc.Height = mInfo.HeightmapHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &mHeightMap[0];
	data.SysMemPitch = mInfo.HeightmapWidth * sizeof(float);
	data.SysMemSlicePitch = 0;

	ID3D11Texture2D* mapTex;
	HR(device->CreateTexture2D(&texDesc, &data, &mapTex));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	HR(device->CreateShaderResourceView(mapTex, &srvDesc, &mHeightMapSRV));

	ReleaseCOM(mapTex);
}
