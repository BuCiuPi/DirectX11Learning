#include "CSceneManager.h"

CSceneManager::CSceneManager()
{

}

CSceneManager::~CSceneManager()
{
	Deinit();
}

HRESULT CSceneManager::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	HRESULT hr;

	mVSConstantBuffer.Initialize(device, deviceContext);
	mPSConstantBuffer.Initialize(device, deviceContext);

	mShaderMaterial = new ShaderMaterial();
	ID3DBlob* vsBlob = mShaderMaterial->BuildShader(device, L"DeferredShading.hlsl", VertexShader);
	//ID3DBlob* vsBlob = mShaderMaterial->BuildShader(device, L"BasicUnlit.hlsl", VertexShader);
	mInputLayout = mShaderMaterial->BuildInputLayout(device, vsBlob, InputLayoutDesc::Basic32, ARRAYSIZE(InputLayoutDesc::Basic32));
	mShaderMaterial->BuildShader(device, L"DeferredShading.hlsl", PixelShader);
	//mShaderMaterial->BuildShader(device, L"BasicUnlit.hlsl", PixelShader);

	return S_OK;
}

void CSceneManager::Deinit()
{
}

void CSceneManager::RenderScene(ID3D11DeviceContext* deviceContext, Camera* camera)
{
	deviceContext->IASetInputLayout(mInputLayout);
	deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mShaderMaterial->SetShader(deviceContext);

	mPSConstantBuffer.data.m_vEyePosition = camera->GetPosition();
	mPSConstantBuffer.data.m_fspecExp = 250.0f;
	mPSConstantBuffer.data.m_fSpectIntensity = 0.25f;
	mPSConstantBuffer.ApplyChanges();
	mPSConstantBuffer.PSShaderUpdate(0);

	XMMATRIX viewProjectionMatrix = camera->ViewProj();

	for (int i = 0; i < mGameObjects.size(); ++i)
	{
		XMMATRIX world = mGameObjects[i]->GetWorldMatrix();
		XMMATRIX MVP = world * viewProjectionMatrix;
		mVSConstantBuffer.data.m_world = XMMatrixTranspose(world);
		mVSConstantBuffer.data.m_worldViewProjection = XMMatrixTranspose(MVP);
		mVSConstantBuffer.ApplyChanges();
		mVSConstantBuffer.VSShaderUpdate(0);

		mGameObjects[i]->Draw(viewProjectionMatrix);
	}
}

void CSceneManager::SetVisibleGameObject(std::vector<GameObject*> gameObjects)
{
	mGameObjects = gameObjects;
}
