#include "pch.h"
#include "Interop.h"
#include <wrl.h>
#include <DirectXHelper.h>

namespace DeviceUtils
{
	UINT64 Signal(CComPtr<ID3D12CommandQueue> commandQueue, CComPtr<ID3D12Fence> fence, UINT64& fenceValue)
	{
		UINT64 fenceValueForSignal = ++fenceValue;
		DX::ThrowIfFailed(commandQueue->Signal(fence, fenceValueForSignal));
		return fenceValueForSignal;
	}

	void WaitForFenceValue(CComPtr<ID3D12Fence> fence, UINT64 fenceValue, HANDLE fenceEvent)
	{
		//ask the fence what's it's current value and if's the GPU hasn't updated this value yet, make the CPU wait for it
		//le preguntamos al fence cual es su valor, y si la GPU aun no ha actualizado ese valor, que la CPU espere hasta que llegue
		if (fence->GetCompletedValue() < fenceValue)
		{
			DX::ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));
			WaitForSingleObject(fenceEvent, INFINITE);
		}
	}

	void Flush(CComPtr<ID3D12CommandQueue> commandQueue, CComPtr<ID3D12Fence> fence, UINT64& fenceValue, HANDLE fenceEvent)
	{
		UINT64 fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
		WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);
	}
}