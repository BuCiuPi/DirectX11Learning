#ifndef CONSTANTBUFFER_H
#define CONSTANTBUFFER_H
#include <d3d11.h>
#include "ConstantBufferTypes.h"
#include <wrl/client.h>

template<class T>
class ConstantBuffer
{
private:
	ConstantBuffer(const ConstantBuffer<T>& rhs);

private:

	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	ID3D11DeviceContext* deviceContext = nullptr;

public:
	ConstantBuffer() {}

	T data;

	ID3D11Buffer* Get()const
	{
		return buffer.Get();
	}

	ID3D11Buffer* const* GetAddressOf()const
	{
		return buffer.GetAddressOf();
	}

	HRESULT Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	{
		if (buffer.Get() != nullptr)
			buffer.Reset();

		this->deviceContext = deviceContext;

		D3D11_BUFFER_DESC desc;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.ByteWidth = static_cast<UINT>(sizeof(T) + (16 - (sizeof(T) % 16)));
		desc.StructureByteStride = 0;

		HRESULT hr = device->CreateBuffer(&desc, nullptr, buffer.GetAddressOf());
		return hr;
	}

	void ApplyChanges()
	{
		this->deviceContext->UpdateSubresource(buffer.Get(), 0, nullptr, &data, 0, 0);
	}

	void PSShaderUpdate(int ShaderSlot)
	{
		this->deviceContext->PSSetConstantBuffers(ShaderSlot, 1, buffer.GetAddressOf());
	}

	void VSShaderUpdate(int ShaderSlot)
	{
		this->deviceContext->VSSetConstantBuffers(ShaderSlot, 1, buffer.GetAddressOf());
	}
};

#endif // ConstantBuffer_h__
