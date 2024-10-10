#ifndef CGBUFFER_H
#define CGBUFFER_H

#include "D3DUtil.h" 

class CGBuffer
{
public:
	CGBuffer();
	~CGBuffer();

	HRESULT Init(ID3D11Device* g_pd3dDevice, UINT width, UINT height);
	void Deinit();

	void PreRender(ID3D11DeviceContext* pd3dImmediateContext);
	void PostRender(ID3D11DeviceContext* pdedImmediateContext);
	void PrepareForUnpack(ID3D11DeviceContext* pd3dImmediateContext, Camera* g_Camera);

	ID3D11Texture2D* GetColorTexture() { return m_ColorSpecIntensityRT; }
	ID3D11DepthStencilView* GetDepthDSV() { return m_DepthStencilDSV; }
	ID3D11DepthStencilView* GetDepthReadOnlyDSV() { return m_DepthStencilReadOnlyDSV; }

	ID3D11ShaderResourceView* GetDepthView() { return m_DepthStencilSRV; }
	ID3D11ShaderResourceView* GetColorView() { return m_ColorSpecIntensitySRV; }
	ID3D11ShaderResourceView* GetNormalView() { return m_NormalSRV; }
	ID3D11ShaderResourceView* GetSpecPowerView() { return m_SpecPowerSRV; }
private:

	ID3D11Buffer* m_pGBufferUnpackCB;

	ID3D11Texture2D* m_DepthStencilRT;
	ID3D11Texture2D* m_ColorSpecIntensityRT;
	ID3D11Texture2D* m_NormalRT;
	ID3D11Texture2D* m_SpecPowerRT;

	ID3D11DepthStencilView* m_DepthStencilDSV;
	ID3D11DepthStencilView* m_DepthStencilReadOnlyDSV;

	ID3D11RenderTargetView* m_ColorSpecIntensityRTV;
	ID3D11RenderTargetView* m_NormalRTV;
	ID3D11RenderTargetView* m_SpecPowerRTV;

	ID3D11ShaderResourceView* m_DepthStencilSRV;
	ID3D11ShaderResourceView* m_ColorSpecIntensitySRV;
	ID3D11ShaderResourceView* m_NormalSRV;
	ID3D11ShaderResourceView* m_SpecPowerSRV;

	ID3D11DepthStencilState* m_DepthStencilState;

};

#endif


