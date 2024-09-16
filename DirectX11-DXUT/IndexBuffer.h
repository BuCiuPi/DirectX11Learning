#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H
#include <d3d11.h>
#include <memory>
#include <wrl/client.h>

class IndexBuffer
{
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	UINT bufferSize = 0;
public:
	IndexBuffer() {}

	ID3D11Buffer* Get() const
	{
		return buffer.Get();
	}

	ID3D11Buffer* const* GetAddressOf() const
	{
		return 	buffer.GetAddressOf();
	}

	UINT BufferSize() const
	{
		return this->bufferSize;
	}

	HRESULT Initialize(ID3D11Device* device, DWORD* data, UINT numIndieces)
	{
		if (buffer.Get() != nullptr)
		{
			buffer.Reset();
		}

		this->bufferSize = numIndieces;

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD) * numIndieces;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA indexBufferData;
		ZeroMemory(&indexBufferData, sizeof(indexBufferData));
		indexBufferData.pSysMem = data;

		HRESULT hr = device->CreateBuffer(&indexBufferDesc, &indexBufferData, this->buffer.GetAddressOf());
		return hr;
	}

};

#endif