#include "ShadowMapApplication.h"

#include <fstream>

#include "GeometryGenerator.h"

ShadowMapApplication::ShadowMapApplication(HINSTANCE hinstance) : DirectX11Application(hinstance)
{
	mCamera.SetPosition(0.0f, 2.0f, -15.0f);
	mCamera.CameraSpeed = 5.0f;

	mLightRotationAngle = 0.0f;

	mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mSceneBounds.Radius = sqrtf(10.0f * 10.0f + 15.0f * 15.0f);

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);

	XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));

	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

		XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));
	}

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mGridMat.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mGridMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mGridMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mGridMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mCylinderMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
	mCylinderMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSphereMat.Ambient = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
	mSphereMat.Diffuse = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
	mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	mSphereMat.Reflect = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);

	mBoxMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mBoxMat.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSkullMat.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mSkullMat.Reflect = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
}

bool ShadowMapApplication::Init(int nShowCmd)
{
	if (!D3DApp::Init(nShowCmd))
	{
		return false;
	}
	RenderStates::InitAll(g_pd3dDevice);

	BuildGeometryBuffer();
	BuildSkullGeometryBuffer();
	BuildScreenQuadBuffers();
	BuildConstantBuffer();
	BuildFX();

	return true;
}

void ShadowMapApplication::DrawScene()
{
	mShadowMap->BindDsvAndSetNullRenderTarget(g_pImmediateContext);

	g_pImmediateContext->RSSetState(RenderStates::DepthOnlyRS);
	DrawSceneToShadowMap();
	g_pImmediateContext->RSSetState(0);
	mShadowMap->UnBindDSV(g_pImmediateContext);

	ID3D11RenderTargetView* renderTargets[1] = { g_pRenderTargetView };
	g_pImmediateContext->OMSetRenderTargets(1, renderTargets, g_pDepthStencilView);
	g_pImmediateContext->RSSetViewports(1, &g_pSceneViewport);

	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	g_pImmediateContext->IASetInputLayout(mDisplacementVertexLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	UINT stride = sizeof(Vertex::PosNormalTexTan);
	UINT offset = 0;

	WavePerFrameBuffer pfb;
	for (size_t i = 0; i < 3; i++)
	{
		pfb.gDirLights[i] = mDirLights[i];
	}
	pfb.gEyePosW = mCamera.GetPosition();
	g_pImmediateContext->UpdateSubresource(mPerFrameBuffer, 0, nullptr, &pfb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &mPerFrameBuffer);
	g_pImmediateContext->HSSetConstantBuffers(1, 1, &mPerFrameBuffer);
	g_pImmediateContext->DSSetConstantBuffers(1, 1, &mPerFrameBuffer);
	g_pImmediateContext->PSSetConstantBuffers(1, 1, &mPerFrameBuffer);

	g_pImmediateContext->VSSetShader(mDisplacementVertexShader, nullptr, 0);
	g_pImmediateContext->HSSetShader(mDisplacementHullShader, nullptr, 0);
	g_pImmediateContext->DSSetShader(mDisplacementDomainShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(mDisplacementPixelShader, nullptr, 0);

	g_pImmediateContext->PSSetSamplers(0, 1, &mSamplerLinear);
	g_pImmediateContext->DSSetSamplers(0, 1, &mSamplerLinear);

	g_pImmediateContext->PSSetSamplers(1, 1, &mSamAnisotropic);

	WaveConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(mCamera.View());
	cb.mProjection = XMMatrixTranspose(mCamera.Proj());
	XMVECTOR detBox = XMMatrixDeterminant(g_World);
	cb.mWorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(&detBox, g_World));

	ID3D11ShaderResourceView* skySRV = mSky->CubeMapSRV();
	g_pImmediateContext->PSSetShaderResources(1, 1, &skySRV);

	g_pImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	for (int i = 0; i < 10; ++i)
	{
		cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mCylWorld[i]));
		cb.gTexTransform = XMMatrixTranspose(XMMatrixIdentity());
		cb.gMaterial = mCylinderMat;

		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
		g_pImmediateContext->HSSetConstantBuffers(0, 1, &g_pConstantBuffer);
		g_pImmediateContext->DSSetConstantBuffers(0, 1, &g_pConstantBuffer);
		g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

		g_pImmediateContext->PSSetShaderResources(0, 1, &mBrickSRV);
		g_pImmediateContext->PSSetShaderResources(2, 1, &mBrickNormalTexSRV);
		g_pImmediateContext->DSSetShaderResources(2, 1, &mBrickNormalTexSRV);

		g_pImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
	}

	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->HSSetShader(nullptr, nullptr, 0);
	g_pImmediateContext->DSSetShader(nullptr, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

	ShadowMappingConstantBuffer smcb;

	smcb.mWorld = XMLoadFloat4x4(&mGridWorld);
	smcb.mView = XMMatrixTranspose(mCamera.View());
	smcb.mProjection = XMMatrixTranspose(mCamera.Proj());
	smcb.gTexTransform = XMMatrixTranspose(XMMatrixScaling(8.0f, 10.0f, 1.0f));
	XMVECTOR sdetBox = XMMatrixDeterminant(smcb.mWorld);
	smcb.mWorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(&sdetBox, smcb.mWorld));

	smcb.gMaterial = mGridMat;
	smcb.gShadowTransform = XMMatrixTranspose(smcb.mWorld * XMLoadFloat4x4(&mShadowTransform));

	ID3D11ShaderResourceView* shadowTransformSRV = mShadowMap->DepthMapSRV();
	g_pImmediateContext->PSSetShaderResources(2, 1, &shadowTransformSRV);

	g_pImmediateContext->PSSetSamplers(1, 1, &mSamplerShadow);

	g_pImmediateContext->UpdateSubresource(mShadowMappingConstantBuffer, 0, nullptr, &smcb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &mShadowMappingConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &mShadowMappingConstantBuffer);

	g_pImmediateContext->PSSetShaderResources(0, 1, &mFloorSRV);

	g_pImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

	//Draw Cube

	smcb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mBoxWorld));
	smcb.gTexTransform = XMMatrixTranspose(XMMatrixScaling(2.0f, 1.0f, 1.0f));
	sdetBox = XMMatrixDeterminant(smcb.mWorld);
	smcb.mWorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(&sdetBox, smcb.mWorld));

	smcb.gMaterial = mBoxMat;
	smcb.gShadowTransform = XMMatrixTranspose(XMLoadFloat4x4(&mShadowTransform));

	g_pImmediateContext->UpdateSubresource(mShadowMappingConstantBuffer, 0, nullptr, &smcb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &mShadowMappingConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &mShadowMappingConstantBuffer);

	//g_pImmediateContext->PSSetShaderResources(0, 1, &mBrickSRV);

	g_pImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

	//DrawSkull(stride, offset, smcb, mShadowMappingConstantBuffer);
	// draw sky
	mSky->Draw(g_pImmediateContext, mCamera);

	g_pSwapChain->Present(0, 0);
}

void ShadowMapApplication::DrawSkull(UINT stride, UINT offset, ShadowMappingConstantBuffer& cb, ID3D11Buffer* shadowConstantBuffer)
{
	g_pImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);
	// drawSkull
	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mSkullWorld));
	cb.gTexTransform = XMMatrixTranspose(XMMatrixIdentity());
	cb.gMaterial = mSkullMat;
	cb.gShadowTransform = XMMatrixTranspose(cb.mWorld * XMLoadFloat4x4(&mShadowTransform));

	g_pImmediateContext->UpdateSubresource(shadowConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &shadowConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &shadowConstantBuffer);

	g_pImmediateContext->PSSetShaderResources(0, 1, &nullSRV);

	g_pImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
}

void ShadowMapApplication::DrawEnviroment(ShadowMappingConstantBuffer& cb, ID3D11Buffer* shadowConstantBuffer)
{
	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mBoxWorld));
	cb.gTexTransform = XMMatrixTranspose(XMMatrixIdentity());
	cb.gMaterial = mBoxMat;
	cb.gShadowTransform = XMMatrixTranspose(cb.mWorld * XMLoadFloat4x4(&mShadowTransform));

	g_pImmediateContext->UpdateSubresource(shadowConstantBuffer, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &shadowConstantBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &shadowConstantBuffer);

	g_pImmediateContext->PSSetShaderResources(0, 1, &mBrickSRV);

	g_pImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

	//for (int i = 0; i < 10; ++i)
	//{
	//	// draw sphere

	//	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mSphereWorld[i]));
	//	cb.gTexTransform = XMMatrixTranspose(XMMatrixIdentity());
	//	cb.gMaterial = mSphereMat;
	//	cb.gShadowTransform = XMMatrixTranspose(cb.mWorld * XMLoadFloat4x4(&mShadowTransform));

	//	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	//	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	//	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	//	g_pImmediateContext->PSSetShaderResources(0, 1, &mStoneSRV);

	//	g_pImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
	//}
}

void ShadowMapApplication::DrawScreenQuad()
{
	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	g_pImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R32_UINT, 0);

	// Scale and shift quad to lower-right corner.
	XMMATRIX world(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 1.0f);

	g_pImmediateContext->DrawIndexed(6, 0, 0);
}

void ShadowMapApplication::DrawSceneToShadowMap()
{
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	g_pImmediateContext->IASetInputLayout(mShadowMapInputLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::PosNormalTexTan);
	UINT offset = 0;

	g_pImmediateContext->VSSetShader(mShadowMapVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(mShadowMapPixelShader, nullptr, 0);

	g_pImmediateContext->PSSetSamplers(0, 1, &mSamplerLinear);

	ShadowConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(XMLoadFloat4x4(&mLightView));
	cb.mProjection = XMMatrixTranspose(XMLoadFloat4x4(&mLightProj));

	g_pImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	for (int i = 0; i < 10; ++i)
	{
		cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mCylWorld[i]));
		cb.gTexTransform = XMMatrixTranspose(XMMatrixIdentity());

		g_pImmediateContext->UpdateSubresource(mShadowConstantBuffers, 0, nullptr, &cb, 0, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &mShadowConstantBuffers);

		g_pImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
	}

	cb.mWorld = XMLoadFloat4x4(&mGridWorld);
	cb.gTexTransform = XMMatrixTranspose(XMMatrixScaling(6.0f, 8.0f, 1.0f));
	g_pImmediateContext->UpdateSubresource(mShadowConstantBuffers, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &mShadowConstantBuffers);

	g_pImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mBoxWorld));
	cb.gTexTransform = XMMatrixTranspose(XMMatrixIdentity());
	g_pImmediateContext->UpdateSubresource(mShadowConstantBuffers, 0, nullptr, &cb, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &mShadowConstantBuffers);

	g_pImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

	//for (int i = 0; i < 10; ++i)
	//{
	//	// draw sphere

	//	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mSphereWorld[i]));
	//	cb.gTexTransform = XMMatrixTranspose(XMMatrixIdentity());

	//	g_pImmediateContext->UpdateSubresource(mShadowConstantBuffers, 0, nullptr, &cb, 0, 0);
	//	g_pImmediateContext->VSSetConstantBuffers(0, 1, &mShadowConstantBuffers);

	//	g_pImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
	//}

	//g_pImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
	//g_pImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);
	//// drawSkull
	//cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&mSkullWorld));
	//cb.gTexTransform = XMMatrixTranspose(XMMatrixIdentity());

	//g_pImmediateContext->UpdateSubresource(mShadowConstantBuffers, 0, nullptr, &cb, 0, 0);
	//g_pImmediateContext->VSSetConstantBuffers(0, 1, &mShadowConstantBuffers);

	//g_pImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
}

void ShadowMapApplication::UpdateScene(float dt)
{
	DirectX11Application::UpdateScene(dt);

	mLightRotationAngle += 0.002f * dt;

	XMMATRIX R = XMMatrixRotationY(mLightRotationAngle);
	XMVECTOR lightDir = XMVector3TransformNormal(XMLoadFloat3(&mDirLights[0].Direction), R);
	XMStoreFloat3(&mDirLights[0].Direction, lightDir);

	BuildShadowTransform();
}

void ShadowMapApplication::BuildGeometryBuffer()
{
	MeshData box;
	MeshData grid;
	MeshData sphere;
	MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.5f, 3.0f, 20, 20, cylinder);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset = 0;
	mGridVertexOffset = box.Vertices.size();
	mSphereVertexOffset = mGridVertexOffset + grid.Vertices.size();
	mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

	// Cache the index count of each object.
	mBoxIndexCount = box.Indices.size();
	mGridIndexCount = grid.Indices.size();
	mSphereIndexCount = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset = 0;
	mGridIndexOffset = mBoxIndexCount;
	mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

	UINT totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount =
		mBoxIndexCount +
		mGridIndexCount +
		mSphereIndexCount +
		mCylinderIndexCount;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::PosNormalTexTan> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex = box.Vertices[i].TexC;
		vertices[k].TangentU = box.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].Tex = grid.Vertices[i].TexC;
		vertices[k].TangentU = grid.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].Tex = sphere.Vertices[i].TexC;
		vertices[k].TangentU = sphere.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].Tex = cylinder.Vertices[i].TexC;
		vertices[k].TangentU = cylinder.Vertices[i].TangentU;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNormalTexTan) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(g_pd3dDevice->CreateBuffer(&vbd, &vinitData, &mShapesVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &mShapesIB));
}

void ShadowMapApplication::BuildSkullGeometryBuffer()
{
	std::ifstream fin("Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<Vertex::PosNormalTexTan> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mSkullIndexCount = 3 * tcount;
	std::vector<UINT> indices(mSkullIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNormalTexTan) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(g_pd3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &mSkullIB));
}

void ShadowMapApplication::BuildScreenQuadBuffers()
{
	MeshData quad;

	GeometryGenerator geoGen;
	geoGen.CreateFullscreenQuad(quad);

	std::vector<Vertex::Basic32> vertices(quad.Vertices.size());

	for (UINT i = 0; i < quad.Vertices.size(); ++i)
	{
		vertices[i].Pos = quad.Vertices[i].Position;
		vertices[i].Normal = quad.Vertices[i].Normal;
		vertices[i].Tex = quad.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * quad.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(g_pd3dDevice->CreateBuffer(&vbd, &vinitData, &mScreenQuadVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * quad.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &quad.Indices[0];
	HR(g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &mScreenQuadIB));
}

void ShadowMapApplication::BuildConstantBuffer()
{
	D3D11_BUFFER_DESC bd;
	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WaveConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	HR(g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer));

	D3D11_BUFFER_DESC sbd;
	// Create the constant buffer
	sbd.Usage = D3D11_USAGE_DEFAULT;
	sbd.ByteWidth = sizeof(SkyConstantBuffer);
	sbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	sbd.CPUAccessFlags = 0;
	sbd.MiscFlags = 0;
	sbd.StructureByteStride = 0;

	HR(g_pd3dDevice->CreateBuffer(&sbd, nullptr, &mSkyConstantBuffer));

	D3D11_BUFFER_DESC Shadowbd;
	// Create the constant buffer
	Shadowbd.Usage = D3D11_USAGE_DEFAULT;
	Shadowbd.ByteWidth = sizeof(ShadowConstantBuffer);
	Shadowbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Shadowbd.CPUAccessFlags = 0;
	Shadowbd.MiscFlags = 0;
	Shadowbd.StructureByteStride = 0;

	HR(g_pd3dDevice->CreateBuffer(&Shadowbd, nullptr, &mShadowConstantBuffers));

	D3D11_BUFFER_DESC ShadowMappingbd;
	// Create the constant buffer
	ShadowMappingbd.Usage = D3D11_USAGE_DEFAULT;
	ShadowMappingbd.ByteWidth = sizeof(ShadowMappingConstantBuffer);
	ShadowMappingbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ShadowMappingbd.CPUAccessFlags = 0;
	ShadowMappingbd.MiscFlags = 0;
	ShadowMappingbd.StructureByteStride = 0;
	HR(g_pd3dDevice->CreateBuffer(&ShadowMappingbd, nullptr, &mShadowMappingConstantBuffer));

	D3D11_BUFFER_DESC pfb;

	pfb.Usage = D3D11_USAGE_DEFAULT;
	pfb.ByteWidth = sizeof(PerFrameBuffer);
	pfb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pfb.CPUAccessFlags = 0;
	pfb.MiscFlags = 0;
	pfb.StructureByteStride = 0;
	HR(g_pd3dDevice->CreateBuffer(&pfb, nullptr, &mPerFrameBuffer));

	HR(CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/floor.dds", nullptr, &mFloorSRV));
	HR(CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/stone.dds", nullptr, &mStoneSRV));
	HR(CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/bricks.dds", nullptr, &mBrickSRV));

	HR(CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/floor_nmap.dds", nullptr, &mStoneNormalTexSRV));
	HR(CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/bricks_nmap.dds", nullptr, &mBrickNormalTexSRV));

	//mSky = new Sky(g_pd3dDevice, L"Textures/grasscube1024.dds", 10.0f);
	//mSky = new Sky(g_pd3dDevice, L"Textures/snowcube1024.dds", 10.0f);
	mSky = new Sky(g_pd3dDevice, L"Textures/desertcube1024.dds", 10.0f);
	mShadowMap = new ShadowMap(g_pd3dDevice, SMapSize, SMapSize);

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
	HR(g_pd3dDevice->CreateSamplerState(&sampDesc, &mSamplerLinear));

	D3D11_SAMPLER_DESC sSampDesc = {};
	sSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	sSampDesc.MipLODBias = 0.0f;
	sSampDesc.MaxAnisotropy = 1;
	sSampDesc.MinLOD = 0;
	sSampDesc.MaxLOD = 0;
	sSampDesc.BorderColor[0] = 0.0f;
	sSampDesc.BorderColor[1] = 0.0f;
	sSampDesc.BorderColor[2] = 0.0f;
	sSampDesc.BorderColor[3] = 0.0f;
	HR(g_pd3dDevice->CreateSamplerState(&sSampDesc, &mSamplerShadow));

	D3D11_SAMPLER_DESC sampAni = {};
	sampAni.Filter = D3D11_FILTER_ANISOTROPIC;
	sampAni.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampAni.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampAni.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampAni.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampAni.MipLODBias = 0.0f;
	sampAni.MaxAnisotropy = 4;
	sampAni.MinLOD = 0;
	sampAni.MaxLOD = D3D11_FLOAT32_MAX;
	HR(g_pd3dDevice->CreateSamplerState(&sampAni, &mSamAnisotropic));
}

void ShadowMapApplication::BuildShadowTransform()
{
	XMVECTOR lightDir = XMLoadFloat3(&mDirLights[0].Direction);
	XMVECTOR lightPos = -2.0f * mSceneBounds.Radius * lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);

	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

	float l = sphereCenterLS.x - mSceneBounds.Radius;
	float b = sphereCenterLS.y - mSceneBounds.Radius;
	float n = sphereCenterLS.z - mSceneBounds.Radius;
	float r = sphereCenterLS.x + mSceneBounds.Radius;
	float t = sphereCenterLS.y + mSceneBounds.Radius;
	float f = sphereCenterLS.z + mSceneBounds.Radius;
	XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	);

	XMMATRIX S = V * P * T;

	XMStoreFloat4x4(&mLightView, V);
	XMStoreFloat4x4(&mLightProj, P);
	XMStoreFloat4x4(&mShadowTransform, S);
}

void ShadowMapApplication::BuildFX()
{
	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"BasicShadowMapping.fxh", "VS", "vs_5_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return;
	}

	InputLayouts::BuildVertexLayout(g_pd3dDevice, pVSBlob, InputLayoutDesc::PosNormalTexTan, ARRAYSIZE(InputLayoutDesc::PosNormalTexTan), &g_pVertexLayout);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"BasicShadowMapping.fxh", "PS", "ps_5_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return;
	////////////////////////////// Normal Displacement

	ID3DBlob* dispVSBlob = nullptr;
	hr = CompileShaderFromFile(L"DisplacementNormalMapping.fxh", "VS", "vs_5_0", &dispVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(dispVSBlob->GetBufferPointer(), dispVSBlob->GetBufferSize(), nullptr, &mDisplacementVertexShader);
	if (FAILED(hr))
	{
		dispVSBlob->Release();
		return;
	}

	InputLayouts::BuildVertexLayout(g_pd3dDevice, dispVSBlob, InputLayoutDesc::PosNormalTexTan, ARRAYSIZE(InputLayoutDesc::PosNormalTexTan), &mDisplacementVertexLayout);
	if (FAILED(hr))
	{
		dispVSBlob->Release();
		return;
	}

	// Compile the hull shader
	ID3DBlob* pHSBlob = nullptr;
	hr = CompileShaderFromFile(L"DisplacementNormalMapping.fxh", "HS", "hs_5_0", &pHSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the Hull shader
	hr = g_pd3dDevice->CreateHullShader(pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), nullptr, &mDisplacementHullShader);
	pHSBlob->Release();
	if (FAILED(hr))
		return;

	// Compile the Domain shader
	ID3DBlob* pDSBlob = nullptr;
	hr = CompileShaderFromFile(L"DisplacementNormalMapping.fxh", "DS", "ds_5_0", &pDSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the Domain shader
	hr = g_pd3dDevice->CreateDomainShader(pDSBlob->GetBufferPointer(), pDSBlob->GetBufferSize(), nullptr, &mDisplacementDomainShader);
	pDSBlob->Release();
	if (FAILED(hr))
		return;

	// Compile the pixel shader
	ID3DBlob* dispPSBlob = nullptr;
	hr = CompileShaderFromFile(L"DisplacementNormalMapping.fxh", "PS", "ps_5_0", &dispPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(dispPSBlob->GetBufferPointer(), dispPSBlob->GetBufferSize(), nullptr, &mDisplacementPixelShader);
	dispPSBlob->Release();
	if (FAILED(hr))
		return;

	ID3DBlob* spVSBlob = nullptr;
	hr = CompileShaderFromFile(L"BuildShadowMap.fxh", "VS", "vs_5_0", &spVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(spVSBlob->GetBufferPointer(), spVSBlob->GetBufferSize(), nullptr, &mShadowMapVertexShader);
	if (FAILED(hr))
	{
		spVSBlob->Release();
		return;
	}

	InputLayouts::BuildVertexLayout(g_pd3dDevice, spVSBlob, InputLayoutDesc::PosNormalTexTan, ARRAYSIZE(InputLayoutDesc::PosNormalTexTan), &mShadowMapInputLayout);
	if (FAILED(hr))
	{
		spVSBlob->Release();
		return;
	}

	// Compile the pixel shader
	ID3DBlob* spPSBlob = nullptr;
	hr = CompileShaderFromFile(L"BuildShadowMap.fxh", "PS", "ps_5_0", &spPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(spPSBlob->GetBufferPointer(), spPSBlob->GetBufferSize(), nullptr, &mShadowMapPixelShader);
	spPSBlob->Release();
	if (FAILED(hr))
		return;

	mSky->BuildSkyFX(g_pd3dDevice);
}

void ShadowMapApplication::CleanupDevice()
{
	DirectX11Application::CleanupDevice();
	SafeDelete(mSky);
	SafeDelete(mShadowMap);

	ReleaseCOM(mShapesVB);
	ReleaseCOM(mShapesIB);
	ReleaseCOM(mSkullVB);
	ReleaseCOM(mSkullIB);
	ReleaseCOM(mFloorSRV);
	ReleaseCOM(mStoneSRV);
	ReleaseCOM(mBrickSRV);
	ReleaseCOM(mStoneNormalTexSRV);
	ReleaseCOM(mBrickNormalTexSRV);
}