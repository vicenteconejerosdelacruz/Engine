#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>

namespace DirectX
{
	static const XMVECTORF32 g_vFLTMAX = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
	static const XMVECTORF32 g_vFLTMIN = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };
	static const XMVECTORF32 g_vHalfVector = { 0.5f, 0.5f, 0.5f, 0.5f };
	static const XMVECTORF32 g_vMultiplySetzwToZero = { 1.0f, 1.0f, 0.0f, 0.0f };
	static const XMVECTORF32 g_vZero = { 0.0f, 0.0f, 0.0f, 0.0f };
}

inline bool IsPowerOfTwo(unsigned int n) { return (n > 0) && ((n & (n - 1)) == 0); }
inline unsigned int PrevPowerOfTwo(unsigned int x) { x = x | (x >> 1); x = x | (x >> 2); x = x | (x >> 4); x = x | (x >> 8); x = x | (x >> 16); return x - (x >> 1); }
inline unsigned int GetMipMaps(unsigned int width, unsigned int height)
{
	using namespace std;
	unsigned int maxD = max(width, height);
	return  1U + std::popcount(maxD - 1);
}

inline XMFLOAT3 ToXMFLOAT3(nlohmann::json f3)
{
	return XMFLOAT3(f3.at(0), f3.at(1), f3.at(2));
}

inline nlohmann::json FromXMFLOAT3(XMFLOAT3 f3)
{
	return nlohmann::json::array({ f3.x,f3.y,f3.z });
}

inline BoundingBox GetContainedBoundingBox(auto objects)
{
	BoundingBox bb;
	bool bbfirst = true;
	for (auto& o : objects)
	{
		if (bbfirst)
		{
			bb = o->GetBoundingBox();
			bbfirst = false;
		}
		else
		{
			BoundingBox obb = o->GetBoundingBox();
			BoundingBox mbb;
			bb.CreateMerged(mbb, obb, bb);
			bb = mbb;
		}
	}
	return bb;
}

inline XMFLOAT4X4 GetBoundindBoxesCentroid(auto& objects)
{
	BoundingBox bb = GetContainedBoundingBox(objects);
	XMVECTOR vec = XMLoadFloat3(&bb.Center);
	XMMATRIX cmx = XMMatrixTranslationFromVector(vec);
	XMFLOAT4X4 w;
	XMStoreFloat4x4(&w, cmx);
	return w;
}

inline void MatrixDump(DirectX::XMMATRIX m)
{
	std::string row1 = "[" +
		std::to_string(m.r[0].m128_f32[0]) + "," +
		std::to_string(m.r[0].m128_f32[1]) + "," +
		std::to_string(m.r[0].m128_f32[2]) + "," +
		std::to_string(m.r[0].m128_f32[3]) + "]";
	std::string row2 = "[" +
		std::to_string(m.r[1].m128_f32[0]) + "," +
		std::to_string(m.r[1].m128_f32[1]) + "," +
		std::to_string(m.r[1].m128_f32[2]) + "," +
		std::to_string(m.r[1].m128_f32[3]) + "]";
	std::string row3 = "[" +
		std::to_string(m.r[2].m128_f32[0]) + "," +
		std::to_string(m.r[2].m128_f32[1]) + "," +
		std::to_string(m.r[2].m128_f32[2]) + "," +
		std::to_string(m.r[2].m128_f32[3]) + "]";
	std::string row4 = "[" +
		std::to_string(m.r[3].m128_f32[0]) + "," +
		std::to_string(m.r[3].m128_f32[1]) + "," +
		std::to_string(m.r[3].m128_f32[2]) + "," +
		std::to_string(m.r[3].m128_f32[3]) + "]";
	std::string matrixDump = row1 + "\n" + row2 + "\n" + row3 + "\n" + row4 + "\n";

	OutputDebugStringA("Matrix\n");
	OutputDebugStringA(matrixDump.c_str());
}

inline void MatrixDump(DirectX::XMFLOAT4X4 m)
{
	std::string row1 = "[" +
		std::to_string(m._11) + "," +
		std::to_string(m._12) + "," +
		std::to_string(m._13) + "," +
		std::to_string(m._14) + "]";
	std::string row2 = "[" +
		std::to_string(m._21) + "," +
		std::to_string(m._22) + "," +
		std::to_string(m._23) + "," +
		std::to_string(m._24) + "]";
	std::string row3 = "[" +
		std::to_string(m._31) + "," +
		std::to_string(m._32) + "," +
		std::to_string(m._33) + "," +
		std::to_string(m._34) + "]";
	std::string row4 = "[" +
		std::to_string(m._41) + "," +
		std::to_string(m._42) + "," +
		std::to_string(m._43) + "," +
		std::to_string(m._44) + "]";
	std::string matrixDump = row1 + "\n" + row2 + "\n" + row3 + "\n" + row4 + "\n";

	OutputDebugStringA("Matrix\n");
	OutputDebugStringA(matrixDump.c_str());
}

inline XMFLOAT3 GetPitchYawRoll(XMFLOAT4X4 transform)
{
	float pitch = XMScalarASin(-transform._32);

	XMVECTOR from(XMVectorSet(transform._12, transform._31, 0.0f, 0.0f));
	XMVECTOR to(XMVectorSet(transform._22, transform._33, 0.0f, 0.0f));
	XMVECTOR res(XMVectorATan2(from, to));

	float roll = XMVectorGetX(res);
	float yaw = XMVectorGetY(res);

	return XMFLOAT3(XMConvertToDegrees(pitch), XMConvertToDegrees(yaw), XMConvertToDegrees(roll));
}

inline std::string OUTPUTV3(XMVECTOR V3)
{
	return std::string(std::to_string(V3.m128_f32[0]) + "," + std::to_string(V3.m128_f32[1]) + "," + std::to_string(V3.m128_f32[2]));
}
