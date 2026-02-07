#pragma once

#include <ppltasks.h>	// Para create_task
#include <Windows.h>
#include <DirectXMath.h>

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Configure un punto de interrupción en esta línea para detectar errores de la API Win32.
			//throw Platform::Exception::CreateException(hr);
			throw std::exception();
		}
	}

	// Función que lee desde un archivo binario de forma asincrónica.
	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::string& filename)
	{
		using namespace Concurrency;

		HANDLE fileHandle = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		LARGE_INTEGER fileSize;
		GetFileSizeEx(fileHandle, &fileSize);
		std::vector<byte> returnBuffer;
		returnBuffer.resize(fileSize.QuadPart);
		DWORD NumberOfBytesRead;
		DX::ThrowIfFailed(ReadFile(fileHandle, &returnBuffer[0], (DWORD)fileSize.QuadPart, &NumberOfBytesRead, nullptr));
		CloseHandle(fileHandle);
		return Concurrency::task_from_result<std::vector<byte>>(returnBuffer);
	}

	/*
	// Convierte una longitud expresada en píxeles independientes del dispositivo (PID) en una longitud expresada en píxeles físicos.
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Redondear al entero más próximo.
	}
*/
// Asignar un nombre al objeto para facilitar la depuración.
#if defined(_DEBUG)
	inline void SetName(ID3D12Object* pObject, LPCWSTR name)
	{
		pObject->SetName(name);
	}

#else
	inline void SetName(ID3D12Object*, LPCWSTR)
	{
	}
#endif
}

// Nombrar función del asistente para ComPtr<T>.
// Asigna el nombre de la variable como nombre del objeto.
#define NAME_D3D12_OBJECT(x) DX::SetName(x.Get(), L#x)
#define CCNAME_D3D12_OBJECT(x) x->SetName(L#x)
#define CCNAME_D3D12_OBJECT_N(x,name) x->SetName(nostd::StringToWString(""###x##":"+name).c_str())
#if defined(_DEBUG)
#define DEBUG_PTR_COUNT(x) OutputDebugStringA(std::string(__FUNCTION__##" -> "  + x->name + "(" + std::to_string(x.use_count()) + ")\n").c_str());
#define DEBUG_PTR_COUNT_JSON(x) OutputDebugStringA(std::string(__FUNCTION__##" -> "  + std::string(x->at("name")) + "(" + std::to_string(x.use_count()) + ")\n").c_str());
#define DEBUG_INSTANCE_REF_COUNT(instanceName,refCountMap,key) OutputDebugStringA(std::string(__FUNCTION__##" -> "  + instanceName + "(" + std::to_string(refCountMap.find(key)->second) + ")\n").c_str());
#else
#define DEBUG_PTR_COUNT(x) 
#define DEBUG_PTR_COUNT_JSON(x)
#define DEBUG_INSTANCE_REF_COUNT(instanceName,refCountMap,key) 
#endif

template<typename T>
void LogCComPtrAddress(std::string name, CComPtr<T> p)
{
#if defined(_DEBUG)
	std::string pAddressS;
	std::stringstream pAddressSStream;
	pAddressSStream << std::setw(16) << std::setfill('0') << std::hex << p.p;
	pAddressSStream >> pAddressS;
	std::string debugS = "Live " + std::string(typeid(T).name()) + "(" + name + ") at 0x" + pAddressS + "\n";
	OutputDebugStringA(debugS.c_str());
#endif
}