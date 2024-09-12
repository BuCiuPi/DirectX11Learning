#include "ParticleSystem.h"

#include <sstream>

ParticleSystem::ParticleSystem()
	: mInitVB(0), mDrawVB(0), mStreamOutVB(0), mTexArraySRV(0), mRandomTexSRV(0)
{

	mFirstRun = true;
	mGameTime = 0.0f;
	mTimeStep = 0.0f;
	mAge = 0.0f;

	mEyePosW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mEmitPosW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mEmitDirW = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

ParticleSystem::~ParticleSystem()
{
	ReleaseCOM(mInitVB);
	ReleaseCOM(mDrawVB);
	ReleaseCOM(mStreamOutVB);
}

float ParticleSystem::GetAge() const
{
	return mAge;
}

void ParticleSystem::SetEyePos(const XMFLOAT3& eyePosW)
{
	mEyePosW = eyePosW;
}

void ParticleSystem::SetEmitPos(const XMFLOAT3& emitPosW)
{
	mEmitPosW = emitPosW;
}

void ParticleSystem::SetEmitDir(const XMFLOAT3& emitDirW)
{
	mEmitDirW = emitDirW;
}

void ParticleSystem::Init(ID3D11Device* device, ID3D11ShaderResourceView* texArraySRV,
	ID3D11ShaderResourceView* randomTexSRV, UINT maxParticles)
{
	mMaxParticle = maxParticles;
	mTexArraySRV = texArraySRV;
	mRandomTexSRV = randomTexSRV;

	BuildVB(device);

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
	sbd.ByteWidth = sizeof(ParticlePerFrameBuffer);
	sbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	sbd.CPUAccessFlags = 0;
	sbd.MiscFlags = 0;
	sbd.StructureByteStride = 0;

	HR(device->CreateBuffer(&sbd, nullptr, &mPerFrameBuffer));

	D3D11_BUFFER_DESC cbd;
	// Create the constant buffer
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(ParticleConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = 0;
	cbd.MiscFlags = 0;
	cbd.StructureByteStride = 0;

	HR(device->CreateBuffer(&cbd, nullptr, &mConstantBuffer));
}

void ParticleSystem::Reset()
{
	mFirstRun = true;
	mAge = 0.0f;
}

void ParticleSystem::Update(float dt, float gameTime)
{
	mGameTime = gameTime;
	mTimeStep = dt;

	mAge += dt;
}

void ParticleSystem::Draw(ID3D11DeviceContext* dc, const Camera& cam)
{
	dc->IASetInputLayout(InputLayouts::Particle);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(Vertex::Particle);
	UINT offset = 0;
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	/// Stream Out Draw
	///

	dc->VSSetShader(StreamOutVS, nullptr, 0);
	dc->GSSetShader(StreamOutGS, nullptr, 0);
	dc->PSSetShader(nullptr, nullptr, 0);
	dc->SOSetTargets(1, &mStreamOutVB, &offset);

	ParticlePerFrameBuffer pfb;
	pfb.gViewProj =	XMMatrixTranspose(cam.ViewProj());
	pfb.gEyePosW = cam.GetPosition();
	pfb.gEmitPosW = mEmitPosW;
	pfb.gEmitDirW = mEmitDirW;
	pfb.gGameTime = mGameTime;
	pfb.gTimeStep = mTimeStep;
	pfb.gFill = 999.0f;

	dc->GSSetShaderResources(0, 1, &mRandomTexSRV);

	dc->OMSetDepthStencilState(RenderStates::DisableDepth, 0);
		dc->GSSetSamplers(0, 1, &mSamplerLinear);

	dc->UpdateSubresource(mPerFrameBuffer, 0, nullptr, &pfb, 0, 0);
	dc->VSSetConstantBuffers(1, 1, &mPerFrameBuffer);
	dc->GSSetConstantBuffers(1, 1, &mPerFrameBuffer);

	if (mFirstRun)
	{
		dc->IASetVertexBuffers(0, 1, &mInitVB, &stride, &offset);

		dc->Draw(1, 0);
		mFirstRun = false;
	}
	else
	{
		dc->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

		dc->DrawAuto();
	}

	ID3D11Buffer* bufferArray[1] = { 0 };
	dc->SOSetTargets(1, bufferArray, &offset);
	std::swap(mDrawVB, mStreamOutVB);

	 //Draw

	dc->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

	dc->VSSetShader(DrawVS, nullptr, 0);
	dc->GSSetShader(DrawGS, nullptr, 0);
	dc->PSSetShader(DrawPS, nullptr, 0);

	ParticleConstantBuffer cb;
	cb.gAccelW = XMFLOAT3(0.0f, 2.0f, 0.0f);

	dc->UpdateSubresource(mConstantBuffer, 0, nullptr, &cb, 0, 0);
	dc->VSSetConstantBuffers(0, 1, &mConstantBuffer);
	dc->GSSetConstantBuffers(0, 1, &mConstantBuffer);
	dc->PSSetConstantBuffers(0, 1, &mConstantBuffer);

	dc->PSSetShaderResources(0, 1, &mTexArraySRV);

	dc->UpdateSubresource(mPerFrameBuffer, 0, nullptr, &pfb, 0, 0);
	dc->VSSetConstantBuffers(1, 1, &mPerFrameBuffer);
	dc->GSSetConstantBuffers(1, 1, &mPerFrameBuffer);

	dc->OMSetDepthStencilState(RenderStates::NoDepthWrite, 0);
	dc->OMSetBlendState(RenderStates::AdditiveBlendingBS, blendFactor, 0xffffffff);

	dc->DrawAuto();

	dc->OMSetBlendState(0, blendFactor, 0xffffffff);
	dc->RSSetState(0);
	dc->OMSetDepthStencilState(0, 0);
	dc->GSSetShader(nullptr, nullptr, 0);
}

void ParticleSystem::BuildVB(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(Vertex::Particle);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	Vertex::Particle p;
	ZeroMemory(&p, sizeof(Vertex::Particle));
	p.Size = XMFLOAT2(1.0f, 1.0f);
	p.Age = 0.0f;
	p.Type = 0;

	D3D11_SUBRESOURCE_DATA vInitdata;
	vInitdata.pSysMem = &p;

	HR(device->CreateBuffer(&vbd, &vInitdata, &mInitVB));

	vbd.ByteWidth = sizeof(Vertex::Particle) * mMaxParticle;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

	HR(device->CreateBuffer(&vbd, 0, &mDrawVB));
	HR(device->CreateBuffer(&vbd, 0, &mStreamOutVB));

}


void ParticleSystem::BuildStreamOutFX(ID3D11Device* device)
{
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"StreamOutParticle.fxh", "StreamOutVS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
		           L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return ;
	}

	// Create the vertex shader
	hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &StreamOutVS);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return ;
	}

	// Compile the Geometry shader
	ID3DBlob* pGSBlob = nullptr;
	hr = CompileShaderFromFile(L"StreamOutParticle.fxh", "StreamOutGS", "gs_4_0", &pGSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
		           L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return ;
	}

	// Create the Geometry shader
	D3D11_SO_DECLARATION_ENTRY pDecl[] =
	{
		// semantic name, semantic index, start component, component count, output slot
		{0, "POSITION", 0, 0, 3, 0 },  
		{0, "VELOCITY", 0, 0, 3, 0 },   
		{0, "SIZE", 0, 0, 2, 0 },    
		{0, "AGE", 0, 0, 1, 0 },    
		{0, "TYPE", 0, 0, 1, 0 }   
	};
	hr = device->CreateGeometryShaderWithStreamOutput(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), pDecl, _countof(pDecl), NULL, 0, 0, NULL, &StreamOutGS );
	pGSBlob->Release();
	if (FAILED(hr))
		return ;
	return ;
}

void ParticleSystem::BuildFX(ID3D11Device* device, LPCTSTR DrawShaderFileName)
{
	if (BuildDrawFX(device, DrawShaderFileName)) return;

	BuildStreamOutFX(device);
	return;
}

bool ParticleSystem::BuildDrawFX(ID3D11Device* device, LPCTSTR DrawShaderFileName)
{
	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(DrawShaderFileName, "DrawVS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
		           L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return true;
	}

	// Create the vertex shader
	hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &DrawVS);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return true;
	}

	InputLayouts::BuildVertexLayout(device, pVSBlob, InputLayoutDesc::Particle, ARRAYSIZE(InputLayoutDesc::Particle), &InputLayouts::Particle);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return true;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(DrawShaderFileName, "DrawPS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
		           L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return true;
	}

	// Create the pixel shader
	hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &DrawPS);
	pPSBlob->Release();
	if (FAILED(hr))
		return true;

	// Compile the Geometry shader
	ID3DBlob* pGSBlob = nullptr;
	hr = CompileShaderFromFile(DrawShaderFileName, "DrawGS", "gs_4_0", &pGSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
		           L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return true;
	}

	// Create the Geometry shader
	hr = device->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &DrawGS);
	pGSBlob->Release();
	if (FAILED(hr))
		return true;
	return false;
}