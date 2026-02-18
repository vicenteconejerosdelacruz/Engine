#include "pch.h"
#include "Cone.h"

namespace Primitives
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <ConeAtt.h>
#include <JEnd.h>

#endif

	Cone::Cone(nlohmann::json& json) : Primitive(json)
	{
#include <Attributes/JInit.h>
#include <ConeAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <ConeAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void Cone::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <ConeAtt.h>
#include <JEnd.h>
		j.erase("uuid");
	}
#endif

	void Cone::PrepareMesh()
	{
		float dr = XM_2PI / static_cast<float>(spread());
		float r = 0.0f;

		XMVECTOR v = { 0.0f, 0.5f, 1.0f, 0.0f };
		XMVECTOR n = { 0.0f, 0.5f, -1.0f, 0.0f };
		XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, dr);

		unsigned int steps = spread() * 2 + 1;
		for (unsigned int i = 0; i < steps; i++)
		{
			XMVECTOR v2 = XMVector3Rotate(v, rotQ);
			vertices.push_back({ .Position = *(XMFLOAT3*)v.m128_f32, .Normal = *(XMFLOAT3*)n.m128_f32 });
			n = XMVector3Rotate(n, rotQ);
			vertices.push_back({ .Position = *(XMFLOAT3*)v2.m128_f32, .Normal = *(XMFLOAT3*)n.m128_f32 });
			vertices.push_back({ .Position = {0.0f,0.0f,0.0f}, .Normal = {0.0f, 0.0f, -1.0f } });

			indices.push_back(static_cast<unsigned int>(vertices.size() - 1ULL));
			indices.push_back(static_cast<unsigned int>(vertices.size() - 3ULL));
			indices.push_back(static_cast<unsigned int>(vertices.size() - 2ULL));

			vertices.push_back({ .Position = *(XMFLOAT3*)v.m128_f32, .Normal = {0.0f, 0.0f, 1.0f } });
			vertices.push_back({ .Position = *(XMFLOAT3*)v2.m128_f32, .Normal = {0.0f, 0.0f, 1.0f } });
			vertices.push_back({ .Position = { 0.0f , 0.0f ,1.0f }, .Normal = {0.0f, 0.0f, 1.0f } });

			v = v2;

			indices.push_back(static_cast<unsigned int>(vertices.size() - 1ULL));
			indices.push_back(static_cast<unsigned int>(vertices.size() - 3ULL));
			indices.push_back(static_cast<unsigned int>(vertices.size() - 2ULL));
		}
	}

	std::vector<uint32_t> Cone::GetIndices()
	{
		return indices;
	}

	std::vector<Cone::VertexType> Cone::GetVertices()
	{
		return vertices;
	}
}