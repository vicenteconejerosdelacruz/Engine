#pragma once
#include <SimpleMath.h>
#include <corecrt_math_defines.h>
#include <foundation/PxVec3.h>
#include <foundation/PxQuat.h>

namespace DirectX
{
	static const XMVECTORF32 g_vFLTMAX = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
	static const XMVECTORF32 g_vFLTMIN = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };
	static const XMVECTORF32 g_vHalfVector = { 0.5f, 0.5f, 0.5f, 0.5f };
	static const XMVECTORF32 g_vMultiplySetzwToZero = { 1.0f, 1.0f, 0.0f, 0.0f };
	static const XMVECTORF32 g_vZero = { 0.0f, 0.0f, 0.0f, 0.0f };
}
using namespace DirectX;
using namespace physx;

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

inline XMFLOAT3 ToXMFLOAT3(PxVec3 v)
{
	return XMFLOAT3(v.x, v.y, v.z);
}

inline XMFLOAT3 ToXMFLOAT3(PxVec3d v)
{
	return XMFLOAT3(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z));
}

inline nlohmann::json FromXMFLOAT3(XMFLOAT3 f3)
{
	return nlohmann::json::array({ f3.x,f3.y,f3.z });
}

inline XMFLOAT4 ToXMFLOAT4(nlohmann::json f4)
{
	return XMFLOAT4(f4.at(0), f4.at(1), f4.at(2), f4.at(3));
}

inline nlohmann::json FromXMFLOAT4(XMFLOAT4 f4)
{
	return nlohmann::json::array({ f4.x,f4.y,f4.z,f4.w });
}

inline XMVECTOR ToXMVECTOR(nlohmann::json xmf4)
{
	return { xmf4.at(0), xmf4.at(1), xmf4.at(2), xmf4.at(3) };
}

inline XMVECTOR ToXMVECTOR(PxVec3 v)
{
	return { v.x, v.y, v.z, 0.0f };
}

inline nlohmann::json FromXMVECTOR(XMVECTOR xmf4)
{
	return nlohmann::json::array({ xmf4.m128_f32[0], xmf4.m128_f32[1], xmf4.m128_f32[2], xmf4.m128_f32[3] });
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

inline XMFLOAT3 Quaternion2Euler(XMVECTOR Q)
{
	XMFLOAT3 angles;

	float x = Q.m128_f32[0];
	float y = Q.m128_f32[1];
	float z = Q.m128_f32[2];
	float w = Q.m128_f32[3];

	float sqw = w * w;
	float sqx = x * x;
	float sqy = y * y;
	float sqz = z * z;
	float unit = sqx + sqy + sqz + sqw; // if normalized is one, otherwise
	// is correction factor
	float test = x * y + z * w;
	if (test > 0.499 * unit) { // singularity at north pole
		angles.y = 2 * std::atan2(x, w);
		angles.z = static_cast<float>(M_PI_2);
		angles.x = 0;
	}
	else if (test < -0.499 * unit) { // singularity at south pole
		angles.y = -2 * std::atan2(x, w);
		angles.z = -static_cast<float>(M_PI_2);
		angles.x = 0;
	}
	else {
		angles.y = std::atan2(2 * y * w - 2 * x * z, sqx - sqy - sqz + sqw); // roll or heading 
		angles.z = std::asin(2 * test / unit); // pitch or attitude
		angles.x = std::atan2(2 * x * w - 2 * y * z, -sqx + sqy - sqz + sqw); // yaw or bank
	}
	angles.x = XMConvertToDegrees(angles.x);
	angles.y = XMConvertToDegrees(angles.y);
	angles.z = XMConvertToDegrees(angles.z);
	return angles;
}

inline PxVec3 ToPxVec3(XMFLOAT3 v)
{
	return PxVec3(v.x, v.y, v.z);
}

inline PxVec3 ToPxVec3(XMVECTOR v)
{
	return PxVec3(v.m128_f32[0], v.m128_f32[1], v.m128_f32[2]);
}

inline PxVec3d ToPxVec3d(XMFLOAT3 v)
{
	PxVec3d vd;
	vd.x = v.x;
	vd.y = v.y;
	vd.z = v.z;
	return vd;
}

inline PxQuat ToPxQuat(XMVECTOR Q)
{
	return PxQuat(Q.m128_f32[0], Q.m128_f32[1], Q.m128_f32[2], Q.m128_f32[3]);
}

inline PxQuat ToPxQuat(XMFLOAT3 euler)
{
	float roll, pitch, yaw;
	pitch = euler.x; yaw = euler.y; roll = euler.z;
	return ToPxQuat(XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll)));
}

inline XMVECTOR XMQuatFromDegrees(float pitch, float yaw, float roll)
{
	return XMQuaternionRotationRollPitchYaw(
		XMConvertToRadians(pitch),
		XMConvertToRadians(yaw),
		XMConvertToRadians(roll)
	);
}

inline XMVECTOR XMQuatFromDegrees(XMFLOAT3 Euler)
{
	return XMQuaternionRotationRollPitchYaw(
		XMConvertToRadians(Euler.x),
		XMConvertToRadians(Euler.y),
		XMConvertToRadians(Euler.z)
	);
}

inline XMVECTOR XMQuatFromDegrees(XMVECTOR Euler)
{
	return XMQuaternionRotationRollPitchYaw(
		XMConvertToRadians(Euler.m128_f32[0]),
		XMConvertToRadians(Euler.m128_f32[1]),
		XMConvertToRadians(Euler.m128_f32[2])
	);
}

inline XMFLOAT3 XMClamp(XMFLOAT3 v, float min, float max)
{
	v.x = std::clamp(v.x, min, max);
	v.y = std::clamp(v.y, min, max);
	v.z = std::clamp(v.z, min, max);
	return v;
}