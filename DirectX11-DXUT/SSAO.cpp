#include "SSAO.h"
#include "D3DUtil.h"
#include "DirectXPackedVector.h"

SSAO::SSAO(ID3D11Device* device, ID3D11DeviceContext* dc, int width, int height, float fovy, float farZ)
	: md3dDevice(device), mDC(dc), mScreenQuadVB(0), mScreenQuadIB(0), mRandomVectorSRV(0),
	mNormalDepthRTV(0), mNormalDepthSRV(0), mAmbientRTV0(0), mAmbientSRV0(0), mAmbientRTV1(0), mAmbientSRV1(0)
{
	OnSize(width, height, fovy, farZ);

	BuildFullScreenQuad();
	BuildOffsetVectors();

	BuildNormalDepthFX();
	BuildSSAOBlurFX();
	BuildRandomVectorTexture();
	BuildNormalDepthConstantBuffer();
}

SSAO::~SSAO()
{
	ReleaseCOM(mScreenQuadIB);
	ReleaseCOM(mScreenQuadVB);
	ReleaseCOM(mRandomVectorSRV);

	ReleaseTextureViews();
}

ID3D11ShaderResourceView* SSAO::NormalDepthSRV()
{
	return mNormalDepthSRV;
}

ID3D11ShaderResourceView* SSAO::AmbientSRV()
{
	return mAmbientSRV0;
}

void SSAO::OnSize(int width, int height, float fovy, float farZ)
{
	mRenderTargetWidth = width;
	mRenderTargetHeight = height;

	mAmbientMapViewport.TopLeftX = 0.0f;
	mAmbientMapViewport.TopLeftY = 0.0f;
	mAmbientMapViewport.Width = width / 2.0f;
	mAmbientMapViewport.Height = height / 2.0f;
	mAmbientMapViewport.MinDepth = 0.0f;
	mAmbientMapViewport.MaxDepth = 1.0f;

	BuildFrushtumFarCorners(fovy, farZ);
	BuildTextureViews();
}

void SSAO::SetNormalDepthRenderTarget(ID3D11DepthStencilView* dsv)
{
	ID3D11RenderTargetView* renderTargets[1] = { mNormalDepthRTV };
	mDC->OMSetRenderTargets(1, renderTargets, dsv);

	float clearColor[] = { 0.0f, 0.0f, -1.0f, 1e5f };
	mDC->ClearRenderTargetView(mNormalDepthRTV, clearColor);
}

void SSAO::ComputeSSAO(const Camera& camera)
{
	ID3D11RenderTargetView* renderTargets[1] = { mAmbientRTV0 };
	mDC->OMSetRenderTargets(1, renderTargets, 0);
	mDC->ClearRenderTargetView(mAmbientRTV0, reinterpret_cast<const float*>(&Colors::Black));
	mDC->RSSetViewports(1, &mAmbientMapViewport);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	mDC->IASetInputLayout(InputLayouts::Basic32);
	mDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mDC->VSSetShader(mNormalDepthVertexShader, nullptr, 0);
	mDC->PSSetShader(mNormalDepthPixelShader, nullptr, 0);

	static const XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	);

	XMMATRIX P = camera.Proj();
	XMMATRIX PT = XMMatrixMultiply(P, T);

	ndcb.gViewToTexSpace = XMMatrixTranspose(PT);
	mDC->UpdateSubresource(mNormalDepthConstantBuffer, 0, nullptr, &ndcb, 0, 0);
	mDC->VSSetConstantBuffers(0, 1, &mNormalDepthConstantBuffer);
	mDC->PSSetConstantBuffers(0, 1, &mNormalDepthConstantBuffer);

	mDC->PSSetShaderResources(0, 1, &mNormalDepthSRV);
	mDC->PSSetShaderResources(1, 1, &mRandomVectorSRV);

	mDC->PSSetSamplers(0, 1, &mSamNormalDepth);
	mDC->PSSetSamplers(1, 1, &mSamRandomVec);

	mDC->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	mDC->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R16_UINT, 0);

	mDC->DrawIndexed(6, 0, 0);
}

void SSAO::BlurAmientMap(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV, bool horzBlur)
{
	ID3D11RenderTargetView* renderTargetView[1] = { outputRTV };
	mDC->OMSetRenderTargets(1, renderTargetView, 0);

	mDC->ClearRenderTargetView(outputRTV, reinterpret_cast<const float*>(&Colors::Black));
	mDC->RSSetViewports(1, &mAmbientMapViewport);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	mDC->IASetInputLayout(InputLayouts::Basic32);
	mDC->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mDC->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	mDC->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R16_UINT, 0);

	mDC->VSSetShader(mSSAOBlurVertexShader, nullptr, 0);
	mDC->PSSetShader(mSSAOBlurPixelShader, nullptr, 0);

	if (horzBlur)
	{
		sbcb.gTexel = XMFLOAT2(1.0f / mAmbientMapViewport.Width, 0.0f);
	}
	else
	{
		sbcb.gTexel = XMFLOAT2(0.0f, 1.0f / mAmbientMapViewport.Height );
	}
	mDC->UpdateSubresource(mSSAOBlurConstantBuffer, 0, nullptr, &sbcb, 0, 0);
	mDC->PSSetConstantBuffers(0, 1, &mSSAOBlurConstantBuffer);

	mDC->PSSetShaderResources(0, 1, &mNormalDepthSRV);
	mDC->PSSetShaderResources(1, 1, &inputSRV);

	mDC->PSSetSamplers(0, 1, &mSamSSAOBlur);

	mDC->DrawIndexed(6, 0, 0);

}

void SSAO::BlurAmientMap(int blurCount)
{
	for (int i = 0; i < blurCount; ++i)
	{
		BlurAmientMap(mAmbientSRV0, mAmbientRTV1, true);
		BlurAmientMap(mAmbientSRV1, mAmbientRTV0, false);
	}
}

void SSAO::BuildFrushtumFarCorners(float fovy, float farZ)
{
	float aspect = (float)mRenderTargetWidth / (float)mRenderTargetHeight;

	float halfHeight = farZ * tanf(0.5f * fovy);
	float halfWidth = aspect * halfHeight;

	ndcb.gFrustumCorners[0] = XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	ndcb.gFrustumCorners[1] = XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	ndcb.gFrustumCorners[2] = XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);
	ndcb.gFrustumCorners[3] = XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);
}

void SSAO::BuildFullScreenQuad()
{
	Vertex::Basic32 v[4];

	v[0].Pos = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	v[1].Pos = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	v[2].Pos = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	v[3].Pos = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	// Store far plane frustum corner indices in Normal.x slot.
	v[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[1].Normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
	v[2].Normal = XMFLOAT3(2.0f, 0.0f, 0.0f);
	v[3].Normal = XMFLOAT3(3.0f, 0.0f, 0.0f);

	v[0].Tex = XMFLOAT2(0.0f, 1.0f);
	v[1].Tex = XMFLOAT2(0.0f, 0.0f);
	v[2].Tex = XMFLOAT2(1.0f, 0.0f);
	v[3].Tex = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;

	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mScreenQuadVB));

	USHORT indices[6] =
	{
		0, 1, 2,
		0, 2, 3
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * 6;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mScreenQuadIB));
}

void SSAO::BuildTextureViews()
{
	ReleaseTextureViews();

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mRenderTargetWidth;
	texDesc.Height = mRenderTargetHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* normalDepthTex = 0;
	HR(md3dDevice->CreateTexture2D(&texDesc, 0, &normalDepthTex));
	HR(md3dDevice->CreateShaderResourceView(normalDepthTex, 0, &mNormalDepthSRV));
	HR(md3dDevice->CreateRenderTargetView(normalDepthTex, 0, &mNormalDepthRTV));

	ReleaseCOM(normalDepthTex);

	texDesc.Width = mRenderTargetWidth / 2;
	texDesc.Height = mRenderTargetHeight / 2;
	texDesc.Format = DXGI_FORMAT_R16_FLOAT;

	ID3D11Texture2D* ambientTex0 = 0;
	HR(md3dDevice->CreateTexture2D(&texDesc, 0, &ambientTex0));
	HR(md3dDevice->CreateShaderResourceView(ambientTex0, 0, &mAmbientSRV0));
	HR(md3dDevice->CreateRenderTargetView(ambientTex0, 0, &mAmbientRTV0));

	ID3D11Texture2D* ambientTex1 = 0;
	HR(md3dDevice->CreateTexture2D(&texDesc, 0, &ambientTex1));
	HR(md3dDevice->CreateShaderResourceView(ambientTex1, 0, &mAmbientSRV1));
	HR(md3dDevice->CreateRenderTargetView(ambientTex1, 0, &mAmbientRTV1));

	ReleaseCOM(ambientTex0);
	ReleaseCOM(ambientTex1);
}

void SSAO::ReleaseTextureViews()
{
	ReleaseCOM(mNormalDepthRTV);
	ReleaseCOM(mNormalDepthSRV);

	ReleaseCOM(mAmbientRTV0);
	ReleaseCOM(mAmbientSRV0);

	ReleaseCOM(mAmbientRTV1);
	ReleaseCOM(mAmbientSRV1);
}

void SSAO::BuildRandomVectorTexture()
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.SysMemPitch = 256 * sizeof(PackedVector::XMCOLOR);

	PackedVector::XMCOLOR color[256 * 256];
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			XMFLOAT3 v(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF());

			color[i * 256 + j] = PackedVector::XMCOLOR(v.x, v.y, v.z, 0.0f);
		}
	}

	initData.pSysMem = color;

	ID3D11Texture2D* tex = 0;
	HR(md3dDevice->CreateTexture2D(&texDesc, &initData, &tex));

	HR(md3dDevice->CreateShaderResourceView(tex, 0, &mRandomVectorSRV));

	// view saves a reference.
	ReleaseCOM(tex);

}

void SSAO::BuildOffsetVectors()
{
	// Start with 14 uniformly distributed vectors.  We choose the 8 corners of the cube
// and the 6 center points along each cube face.  We always alternate the points on 
// opposites sides of the cubes.  This way we still get the vectors spread out even
// if we choose to use less than 14 samples.

// 8 cube corners
	ndcb.gOffsetVectors[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	ndcb.gOffsetVectors[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	ndcb.gOffsetVectors[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	ndcb.gOffsetVectors[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	ndcb.gOffsetVectors[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	ndcb.gOffsetVectors[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	ndcb.gOffsetVectors[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	ndcb.gOffsetVectors[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	ndcb.gOffsetVectors[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	ndcb.gOffsetVectors[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	ndcb.gOffsetVectors[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	ndcb.gOffsetVectors[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	ndcb.gOffsetVectors[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	ndcb.gOffsetVectors[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < 14; ++i)
	{
		// Create random lengths in [0.25, 1.0].
		float s = MathHelper::RandF(0.25f, 1.0f);

		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&ndcb.gOffsetVectors[i]));

		XMStoreFloat4(&ndcb.gOffsetVectors[i], v);
	}

}

void SSAO::DrawFullScreenQuad()
{
}

void SSAO::BuildNormalDepthFX()
{
	// Compile the vertex shader
	ID3DBlob* skyVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"SSAO.fxh", "VS", "vs_5_0", &skyVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the vertex shader
	hr = md3dDevice->CreateVertexShader(skyVSBlob->GetBufferPointer(), skyVSBlob->GetBufferSize(), nullptr, &mNormalDepthVertexShader);
	if (FAILED(hr))
	{
		skyVSBlob->Release();
		return;
	}

	InputLayouts::BuildVertexLayout(md3dDevice, skyVSBlob, InputLayoutDesc::Basic32, ARRAYSIZE(InputLayoutDesc::Basic32), &InputLayouts::Basic32);
	if (FAILED(hr))
	{
		skyVSBlob->Release();
		return;
	}

	// Compile the pixel shader
	ID3DBlob* skyPSBlob = nullptr;
	hr = CompileShaderFromFile(L"SSAO.fxh", "PS", "ps_5_0", &skyPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the pixel shader
	hr = md3dDevice->CreatePixelShader(skyPSBlob->GetBufferPointer(), skyPSBlob->GetBufferSize(), nullptr, &mNormalDepthPixelShader);
	skyPSBlob->Release();
	if (FAILED(hr))
		return;
}

void SSAO::BuildSSAOBlurFX()
{
	// Compile the vertex shader
	ID3DBlob* skyVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"SSAOBlur.fxh", "VS", "vs_5_0", &skyVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the vertex shader
	hr = md3dDevice->CreateVertexShader(skyVSBlob->GetBufferPointer(), skyVSBlob->GetBufferSize(), nullptr, &mSSAOBlurVertexShader);
	if (FAILED(hr))
	{
		skyVSBlob->Release();
		return;
	}

	//InputLayouts::BuildVertexLayout(md3dDevice, skyVSBlob, InputLayoutDesc::Basic32, ARRAYSIZE(InputLayoutDesc::Basic32), &InputLayouts::Basic32);
	if (FAILED(hr))
	{
		skyVSBlob->Release();
		return;
	}

	// Compile the pixel shader
	ID3DBlob* skyPSBlob = nullptr;
	hr = CompileShaderFromFile(L"SSAOBlur.fxh", "PS", "ps_5_0", &skyPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the pixel shader
	hr = md3dDevice->CreatePixelShader(skyPSBlob->GetBufferPointer(), skyPSBlob->GetBufferSize(), nullptr, &mSSAOBlurPixelShader);
	skyPSBlob->Release();
	if (FAILED(hr))
		return;
}

void SSAO::BuildNormalDepthConstantBuffer()
{
	D3D11_BUFFER_DESC bd;
	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(NormalDepthConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	HR(md3dDevice->CreateBuffer(&bd, nullptr, &mNormalDepthConstantBuffer));

	D3D11_BUFFER_DESC sbbd;
	// Create the constant buffer
	sbbd.Usage = D3D11_USAGE_DEFAULT;
	sbbd.ByteWidth = sizeof(SSAOBlurConstantBuffer);
	sbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	sbbd.CPUAccessFlags = 0;
	sbbd.MiscFlags = 0;
	sbbd.StructureByteStride = 0;
	HR(md3dDevice->CreateBuffer(&sbbd, nullptr, &mSSAOBlurConstantBuffer));

	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.BorderColor[0] = 0.0f;
	sampDesc.BorderColor[1] = 0.0f;
	sampDesc.BorderColor[2] = 0.0f;
	sampDesc.BorderColor[3] = 1e5f;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(md3dDevice->CreateSamplerState(&sampDesc, &mSamNormalDepth));

	D3D11_SAMPLER_DESC sampRDesc = {};
	sampRDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sampRDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampRDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampRDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampRDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampRDesc.MipLODBias = 0.0f;
	sampRDesc.MaxAnisotropy = 1;
	sampRDesc.MinLOD = 0;
	sampRDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(md3dDevice->CreateSamplerState(&sampRDesc, &mSamRandomVec));

	D3D11_SAMPLER_DESC samSBDesc = {};
	samSBDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samSBDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samSBDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samSBDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samSBDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samSBDesc.MipLODBias = 0.0f;
	samSBDesc.MaxAnisotropy = 1;
	samSBDesc.MinLOD = 0;
	samSBDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(md3dDevice->CreateSamplerState(&samSBDesc, &mSamSSAOBlur));
}
