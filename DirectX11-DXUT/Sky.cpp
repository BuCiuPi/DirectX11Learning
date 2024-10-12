#include "Sky.h"

#include <complex>

#include "CubeMap.h"
#include "GeometryGenerator.h"
#include "IntegrateBRDFShader.h"
#include "IrradianceShader.h"
#include "ShaderMaterial.h"
#include "ModelTexture.h"
#include "PreFilterShader.h"
#include "RectToCubeMapRenderShader.h"
#include "SkyBoxShader.h"
#include "Texture.h"

Sky::Sky(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& fileName, float skySphereRadius)
{
	HR(CreateDDSTextureFromFile(device, fileName.c_str(), nullptr, &mCubeMapSRV));

	MeshData sphere;
	GeometryGenerator geoGen;
	geoGen.CreateSphere(skySphereRadius, 30, 30, sphere);

	std::vector<XMFLOAT3> vertices(sphere.Vertices.size());

	for (size_t i = 0; i < sphere.Vertices.size(); ++i)
	{
		vertices[i] = sphere.Vertices[i].Position;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(XMFLOAT3) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];

	HR(device->CreateBuffer(&vbd, &vinitData, &mVB));


	mIndexCount = sphere.Indices.size();

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * mIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	std::vector<USHORT> indices16;
	indices16.assign(sphere.Indices.begin(), sphere.Indices.end());

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices16[0];

	HR(device->CreateBuffer(&ibd, &iinitData, &mIB));

	CreateCubeBuffer(device);

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
	HR(device->CreateSamplerState(&sampDesc, &mSamplerLinear));

	mSkyConstantBuffer.Initialize(device, deviceContext);
	mCubeCamera = new Camera(XMFLOAT3(0.0f, 0.0f, 0.0f));
	mSkyBoxShader = new SkyBoxShader();
	mSkyBoxShader->Initialize(device);
}

Sky::~Sky()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mCubeMapSRV);
}

ID3D11ShaderResourceView* Sky::CubeMapSRV()
{
	return  mCubeMapSRV;
}

bool Sky::BuildSkyFX(ID3D11Device* g_pd3dDevice)
{
	// Compile the vertex shader
	mSkyShaderMaterial = new ShaderMaterial();
	ID3DBlob* skyVSBlob = mSkyShaderMaterial->BuildShader(g_pd3dDevice, L"Sky.fxh", VertexShader);
	mSkyShaderMaterial->BuildInputLayout(g_pd3dDevice, skyVSBlob, InputLayoutDesc::Pos, ARRAYSIZE(InputLayoutDesc::Pos));
	mSkyShaderMaterial->BuildShader(g_pd3dDevice, L"Sky.fxh", PixelShader);
	return  true;
}

void Sky::Draw(ID3D11DeviceContext* dc, const Camera& camera)
{
	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;

	//dc->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	//dc->IASetIndexBuffer(mIB, DXGI_FORMAT_R16_UINT, 0);

	BindMesh(dc);

	mSkyShaderMaterial->SetInputLayout(dc);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mSkyShaderMaterial->SetShader(dc);

	XMFLOAT3 camPos = camera.GetPosition();
	XMMATRIX skyWorld = XMMatrixTranslation(camPos.x, camPos.y, camPos.z);
	mSkyConstantBuffer.data.mMVP = XMMatrixTranspose(XMMatrixMultiply(skyWorld, camera.ViewProj()));
	mSkyConstantBuffer.ApplyChanges();
	mSkyConstantBuffer.VSShaderUpdate(0);
	mSkyConstantBuffer.PSShaderUpdate(0);

	dc->PSSetSamplers(0, 1, &mSamplerLinear);
	//dc->PSSetShaderResources(0, 1, &mCubeMapSRV);
	ID3D11ShaderResourceView* srv = mCubeMap->GetSRV();
	//ID3D11ShaderResourceView* srv = mIrradianceMap->GetSRV();
	//ID3D11ShaderResourceView* srv = mPreFilterMap->GetSRV();
	dc->PSSetShaderResources(0, 1, &srv);
	//dc->PSSetShaderResources(1, 1, &mCubeMapSRV);

	dc->OMSetDepthStencilState(RenderStates::LessEqualDSS, 0);
	dc->RSSetState(RenderStates::NoCullRS);

	dc->DrawIndexed(mCubeIndexCount, 0, 0);

	mSkyShaderMaterial->UnSetShader(dc);
	dc->OMSetDepthStencilState(nullptr, 0);
	dc->RSSetState(nullptr);

}

void Sky::Render(ID3D11DeviceContext* dc, const Camera& camera)
{
	BindMesh(dc);

	mSkyCubeConstantBuffer.data.View = XMMatrixTranspose(camera.View());
	mSkyCubeConstantBuffer.data.Projection = XMMatrixTranspose(camera.Proj());
	mSkyCubeConstantBuffer.data.CamPos = mCubeCamera->GetPosition();
	mSkyCubeConstantBuffer.ApplyChanges();

	dc->PSSetSamplers(0, 1, &mSamplerLinear);
	ID3D11ShaderResourceView* srv = mCubeMap->GetSRV();
	dc->PSSetShaderResources(0, 1, &srv);

	mSkyBoxShader->Render(dc, mCubeIndexCount, &mSkyCubeConstantBuffer);

	ID3D11ShaderResourceView* irradiance = mIrradianceMap->GetSRV();
	ID3D11ShaderResourceView* preFilter = mPreFilterMap->GetSRV();
	ID3D11ShaderResourceView* brdfLut = mBrdfLUT->GetSRV();

	dc->PSSetShaderResources(1, 1, &irradiance);
	dc->PSSetShaderResources(2, 1, &preFilter);
	dc->PSSetShaderResources(3, 1, &brdfLut);
}

void Sky::CreateCubeBuffer(ID3D11Device* device)
{
	MeshData cube;
	GeometryGenerator geoGen;
	geoGen.CreateCube(cube, mCubeVertexCount, mCubeIndexCount);

	std::vector<PosUvVertexType> vertices(cube.Vertices.size());

	for (size_t i = 0; i < cube.Vertices.size(); ++i)
	{
		vertices[i].Position = cube.Vertices[i].Position;
		vertices[i].Uv = cube.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(PosUvVertexType) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];

	HR(device->CreateBuffer(&vbd, &vinitData, &mCVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(unsigned long) * mCubeIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;


	std::vector<unsigned long> indices16;
	indices16.assign(cube.Indices.begin(), cube.Indices.end());

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices16[0];

	HR(device->CreateBuffer(&ibd, &iinitData, &mCIB));
}

bool Sky::CreateCubeMap(D3DApp* app, const wchar_t* fileName)
{
	std::vector<RenderTexture*> cubeFace;
	ID3D11Device* device = app->g_pd3dDevice;
	ID3D11DeviceContext* deviceContext = app->g_pImmediateContext;

	Texture* image = new Texture();
	const HRESULT result = image->Initialize(device, fileName);
	if (!result)
	{
		return false;
	}

	for (int i = 0; i < 6; ++i)
	{
		RenderTexture* renderTexture = new RenderTexture();
		renderTexture->Initialize(device, mSkyBoxSize, mSkyBoxSize, 1);
		cubeFace.push_back(renderTexture);
	}

	RectToCubeMapRenderShader* shader = new RectToCubeMapRenderShader();
	if (!shader->Initialize(device))
	{
		return false;
	}
	BindMesh(deviceContext);
	ID3D11ShaderResourceView* srv = image->GetSRV();

	deviceContext->PSSetShaderResources(0, 1, &srv);

	XMFLOAT3 center = mCubeCamera->GetPosition();
	float x = center.x;
	float y = center.y;
	float z = center.z;
	XMFLOAT3 worldUp(0.0f, 1.0f, 0.0f);

	XMFLOAT3 targets[6] =
	{
		XMFLOAT3(x + 1.0f, y, z), // +X
		XMFLOAT3(x - 1.0f, y, z), // -X
		XMFLOAT3(x, y - 1.0f, z), // -Y
		XMFLOAT3(x, y + 1.0f, z), // +Y
		XMFLOAT3(x, y, z - 1.0f),  // -Z
		XMFLOAT3(x, y, z + 1.0f), // +Z
	};

	XMFLOAT3 ups[6] =
	{
		XMFLOAT3(0.0f, -1.0f, 0.0f),  // +X
		XMFLOAT3(0.0f, -1.0f, 0.0f),  // -X
		XMFLOAT3(0.0f, 0.0f, +1.0f), // +Y
		XMFLOAT3(0.0f, 0.0f, -1.0f), // -Y
		XMFLOAT3(0.0f, -1.0f, 0.0f),	 // +Z
		XMFLOAT3(0.0f, -1.0f, 0.0f),	 // -Z
	};

	mSkyCubeConstantBuffer.Initialize(device, deviceContext);

	for (int i = 0; i < 6; ++i)
	{
		RenderTexture* texture = cubeFace[i];

		texture->SetRenderTarget(app, deviceContext);
		texture->ClearRenderTarget(deviceContext, app->g_pDepthStencilView, 0.0f, 0.0f, 0.0f, 1.0f);

		mCubeCamera->LookAt(center, targets[i], ups[i]);
		mCubeCamera->SetLens(0.5f * XM_PI, 1.0f, 0.1f, 1000.0f);
		mCubeCamera->UpdateViewMatrix();

		mSkyCubeConstantBuffer.data.View = XMMatrixTranspose(mCubeCamera->View());
		mSkyCubeConstantBuffer.data.Projection = XMMatrixTranspose(mCubeCamera->Proj());
		mSkyCubeConstantBuffer.data.CamPos = mCubeCamera->GetPosition();
		mSkyCubeConstantBuffer.ApplyChanges();

		shader->Render(deviceContext, mCubeIndexCount, &mSkyCubeConstantBuffer);
	}

	delete shader;
	delete image;

	mCubeMap = new CubeMap;
	if (!mCubeMap->Initialize(device, deviceContext, cubeFace, mSkyBoxSize, mSkyBoxSize, 1))
	{
		return false;
	}

	for (int i = 0; i < 6; ++i)
	{
		delete cubeFace[i];
		RenderTexture* renderTexture = new RenderTexture;
		renderTexture->Initialize(device, mIrradianceSize, mIrradianceSize, 1);
		cubeFace[i] = renderTexture;
	}

	IrradianceShader* irradianceShader = new IrradianceShader;
	if (!irradianceShader->Initialize(device))
	{
		return false;
	}

	srv = mCubeMap->GetSRV();
	deviceContext->PSSetShaderResources(0, 1, &srv);

	for (int i = 0; i < 6; ++i)
	{
		RenderTexture* texture = cubeFace[i];

		texture->SetRenderTarget(app, deviceContext);
		texture->ClearRenderTarget(deviceContext, app->g_pDepthStencilView, 0.0f, 0.0f, 0.0f, 1.0f);

		mCubeCamera->LookAt(center, targets[i], ups[i]);
		mCubeCamera->SetLens(0.5f * XM_PI, 1.0f, 0.1f, 1000.0f);
		mCubeCamera->UpdateViewMatrix();

		mSkyCubeConstantBuffer.data.View = XMMatrixTranspose(mCubeCamera->View());
		mSkyCubeConstantBuffer.data.Projection = XMMatrixTranspose(mCubeCamera->Proj());
		mSkyCubeConstantBuffer.data.CamPos = mCubeCamera->GetPosition();
		mSkyCubeConstantBuffer.ApplyChanges();

		irradianceShader->Render(deviceContext, mCubeIndexCount, &mSkyCubeConstantBuffer);
	}

	delete irradianceShader;

	mIrradianceMap = new CubeMap;
	if (!mIrradianceMap->Initialize(device, deviceContext, cubeFace, mIrradianceSize, mIrradianceSize, 1))
	{
		return false;
	}

	PreFilterShader* preFilterShader = new PreFilterShader;
	if (!preFilterShader->Initialize(device))
	{
		return false;
	}

	mPreFilterMap = new CubeMap;
	if (!mPreFilterMap->Initialize(device, deviceContext, std::vector<RenderTexture*>(), PreFilterSize, PreFilterSize, 5))
	{
		return false;
	}

	for (int mip = 0; mip < 5; ++mip)
	{
		const unsigned int mipWidth = unsigned int(PreFilterSize * pow(0.5, mip));
		const unsigned int mipHeight = unsigned int(PreFilterSize * pow(0.5, mip));

		for (int i = 0; i < 6; ++i)
		{
			delete cubeFace[i];
			RenderTexture* renderTexture = new RenderTexture;
			renderTexture->Initialize(device, mipWidth, mipHeight, 1);
			cubeFace[i] = renderTexture;
		}

		const float roughness = float(mip) / 4.0f;
		mSkyCubeConstantBuffer.data.CustomData = XMFLOAT4(roughness, 0.0f, 0.0f, 0.0f);

		for (int i = 0; i < 6; ++i)
		{
			RenderTexture* texture = cubeFace[i];

			texture->SetRenderTarget(app, deviceContext);
			texture->ClearRenderTarget(deviceContext, app->g_pDepthStencilView, 0.0f, 0.0f, 0.0f, 1.0f);

			mCubeCamera->LookAt(center, targets[i], ups[i]);
			mCubeCamera->SetLens(0.5f * XM_PI, 1.0f, 0.1f, 1000.0f);
			mCubeCamera->UpdateViewMatrix();

			mSkyCubeConstantBuffer.data.View = XMMatrixTranspose(mCubeCamera->View());
			mSkyCubeConstantBuffer.data.Projection = XMMatrixTranspose(mCubeCamera->Proj());
			mSkyCubeConstantBuffer.data.CamPos = mCubeCamera->GetPosition();
			mSkyCubeConstantBuffer.ApplyChanges();

			preFilterShader->Render(deviceContext, mCubeIndexCount, &mSkyCubeConstantBuffer);
		}

		mPreFilterMap->Copy(deviceContext, cubeFace, mipWidth, mipHeight, mip);
	}

	delete preFilterShader;

	for (int i = 0; i < 6; ++i)
	{
		delete cubeFace[i];
	}

	IntegrateBRDFShader* integrateBRDFShader = new IntegrateBRDFShader;
	if (!integrateBRDFShader->Initialize(device))
	{
		return false;
	}

	mBrdfLUT = new RenderTexture;
	if (!mBrdfLUT->Initialize(device, BrdfLookupSize, BrdfLookupSize, 1))
	{
		return false;
	}

	mBrdfLUT->SetRenderTarget(app, deviceContext);
	mBrdfLUT->ClearRenderTarget(deviceContext, app->g_pDepthStencilView, 0.0f, 0.0f, 0.0f, 1.0f);

	mCubeCamera->LookAt(center, XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	mCubeCamera->SetLens(0.5f * XM_PI, 1.0f, 0.1f, 1000.0f);
	mCubeCamera->UpdateViewMatrix();

	mSkyCubeConstantBuffer.data.View = XMMatrixTranspose(mCubeCamera->View());
	mSkyCubeConstantBuffer.data.Projection = XMMatrixTranspose(mCubeCamera->Proj());
	mSkyCubeConstantBuffer.data.CamPos = mCubeCamera->GetPosition();
	mSkyCubeConstantBuffer.ApplyChanges();

	if (!integrateBRDFShader->Render(deviceContext, mCubeIndexCount, &mSkyCubeConstantBuffer))
	{
		return false;
	}

	delete integrateBRDFShader;

	return true;
}

void Sky::BindMesh(ID3D11DeviceContext* deviceContext) const
{
	UINT stride = sizeof(PosUvVertexType);
	UINT offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &mCVB, &stride, &offset);
	deviceContext->IASetIndexBuffer(mCIB, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
