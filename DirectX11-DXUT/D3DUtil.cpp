#include "D3DUtil.h"
#include "SDKmisc.h"

#pragma region InputLayoutDesc

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Basic32[3] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::TreePointSprite[2] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::InstancedBasic32[8] =
{
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
    { "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Pos[1] =
{
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
};


#pragma endregion

#pragma region InputLayouts

ID3D11InputLayout* InputLayouts::Basic32 = 0;
ID3D11InputLayout* InputLayouts::Pos = 0;
ID3D11InputLayout* InputLayouts::TreePointSprite = 0;

HRESULT InputLayouts::BuildVertexLayout(ID3D11Device* device, ID3DBlob* pVSBlob, const D3D11_INPUT_ELEMENT_DESC layout[], UINT numElements, ID3D11InputLayout** inputLayout)
{
	HRESULT hr = device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), inputLayout);
	return hr;
}

void InputLayouts::DestroyAll()
{
	ReleaseCOM(Basic32);
	ReleaseCOM(TreePointSprite);
}

#pragma endregion

HRESULT LoadTextureArray(ID3D11DeviceContext* deviceContex, ID3D11Device* pd3dDevice, LPCTSTR* szTextureNames, int iNumTextures, ID3D11Texture2D** ppTex2D, ID3D11ShaderResourceView** ppSRV)
{
	HRESULT hr = S_OK;
	D3D11_TEXTURE2D_DESC desc = { 0 };

	WCHAR str[MAX_PATH] = {};
	for (int i = 0; i < iNumTextures; i++)
	{
		V_RETURN(DXUTFindDXSDKMediaFileCch(str, MAX_PATH, szTextureNames[i]));

		ID3D11Resource* pRes = nullptr;
		V_RETURN(CreateDDSTextureFromFileEx(pd3dDevice, str, 0, D3D11_USAGE_STAGING,
			0, D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ, 0, DDS_LOADER_FORCE_SRGB,
			&pRes, nullptr, nullptr));
		
		if (pRes)
		{
			ID3D11Texture2D* pTemp;
			V_RETURN(pRes->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID*)&pTemp));
            pTemp->GetDesc(&desc);

            if (desc.MipLevels > 4)
                desc.MipLevels -= 4;
            if (!(*ppTex2D))
            {
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                desc.CPUAccessFlags = 0;
                desc.ArraySize = iNumTextures;
                V_RETURN(pd3dDevice->CreateTexture2D(&desc, nullptr, ppTex2D));
            }

            for (UINT iMip = 0; iMip < desc.MipLevels; iMip++)
            {

                D3D11_MAPPED_SUBRESOURCE mr;

                deviceContex->Map(pTemp, iMip, D3D11_MAP_READ, 0, &mr);

                if (mr.pData)
                {
                    deviceContex->UpdateSubresource((*ppTex2D),
                        D3D11CalcSubresource(iMip, i, desc.MipLevels),
                        nullptr,
                        mr.pData,
                        mr.RowPitch,
                        0);
                }

                deviceContex->Unmap(pTemp, iMip);
            }

            SAFE_RELEASE(pRes);
            SAFE_RELEASE(pTemp);
        }
        else
        {
            return E_FAIL;
        }

	}

    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Format = MAKE_SRGB(desc.Format);
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    SRVDesc.Texture2DArray.MipLevels = desc.MipLevels;
    SRVDesc.Texture2DArray.ArraySize = iNumTextures;
    V_RETURN(pd3dDevice->CreateShaderResourceView(*ppTex2D, &SRVDesc, ppSRV));

	return hr;
}
