#include "Sky.h"
#include "GeometryGenerator.h"

Sky::Sky(ID3D11Device* device, const std::wstring& fileName, float skySphereRadius)
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

	D3D11_BUFFER_DESC sbd;
	// Create the constant buffer
	sbd.Usage = D3D11_USAGE_DEFAULT;
	sbd.ByteWidth = sizeof(SkyConstantBuffer);
	sbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	sbd.CPUAccessFlags = 0;
	sbd.MiscFlags = 0;
	sbd.StructureByteStride = 0;

	HR(device->CreateBuffer(&sbd, nullptr, &mSkyConstantBuffer));
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
	ID3DBlob* skyVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"Sky.fxh", "VS", "vs_5_0", &skyVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return false;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(skyVSBlob->GetBufferPointer(), skyVSBlob->GetBufferSize(), nullptr, &mSkyVertexShader);
	if (FAILED(hr))
	{
		skyVSBlob->Release();
		return false;
	}

	InputLayouts::BuildVertexLayout(g_pd3dDevice, skyVSBlob, InputLayoutDesc::Pos, ARRAYSIZE(InputLayoutDesc::Pos), &InputLayouts::Pos);
	if (FAILED(hr))
	{
		skyVSBlob->Release();
		return false;
	}

	// Compile the pixel shader
	ID3DBlob* skyPSBlob = nullptr;
	hr = CompileShaderFromFile(L"Sky.fxh", "PS", "ps_5_0", &skyPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return false;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(skyPSBlob->GetBufferPointer(), skyPSBlob->GetBufferSize(), nullptr, &mSkyPixelShader);
	skyPSBlob->Release();
	if (FAILED(hr))
		return false;
	return  true;
}

void Sky::Draw(ID3D11DeviceContext* dc, const Camera& camera)
{
	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;

	dc->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	dc->IASetIndexBuffer(mIB, DXGI_FORMAT_R16_UINT, 0);

	dc->IASetInputLayout(InputLayouts::Pos);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	dc->VSSetShader(mSkyVertexShader, nullptr, 0);
	dc->PSSetShader(mSkyPixelShader, nullptr, 0);

	SkyConstantBuffer scb;
	XMFLOAT3 camPos = camera.GetPosition();
	XMMATRIX skyWorld = XMMatrixTranslation(camPos.x, camPos.y, camPos.z);

	scb.mMVP = XMMatrixTranspose(XMMatrixMultiply(skyWorld, camera.ViewProj()));

	dc->UpdateSubresource(mSkyConstantBuffer, 0, nullptr, &scb, 0, 0);
	dc->VSSetConstantBuffers(0, 1, &mSkyConstantBuffer);
	dc->PSSetConstantBuffers(0, 1, &mSkyConstantBuffer);

	dc->PSSetSamplers(0, 1, &mSamplerLinear);
	dc->PSSetShaderResources(0, 1, &mCubeMapSRV);
	dc->PSSetShaderResources(1, 1, &mCubeMapSRV);


	dc->OMSetDepthStencilState(RenderStates::LessEqualDSS, 0);
	dc->RSSetState(RenderStates::NoCullRS);


	dc->DrawIndexed(mIndexCount, 0, 0);

	dc->OMSetDepthStencilState(nullptr, 0);
	dc->RSSetState(nullptr);

}
