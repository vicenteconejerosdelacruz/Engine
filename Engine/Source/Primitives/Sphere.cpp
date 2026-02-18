#include "pch.h"
#include "Sphere.h"
#include <corecrt_math_defines.h>

const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

//non normalized points, don't use directly
static std::vector<XMFLOAT3> icosahedronNNPoints = {
	XMFLOAT3(-1.0f, t, 0.0f),
	XMFLOAT3(1.0f, t, 0.0f),
	XMFLOAT3(-1.0f, -t, 0.0f),
	XMFLOAT3(1.0f, -t, 0.0f),
	XMFLOAT3(0.0, -1.0, t),
	XMFLOAT3(0.0, 1.0, t),
	XMFLOAT3(0.0, -1.0, -t),
	XMFLOAT3(0.0, 1.0, -t),
	XMFLOAT3(t, 0.0, -1.0),
	XMFLOAT3(t, 0.0, 1.0),
	XMFLOAT3(-t, 0.0, -1.0),
	XMFLOAT3(-t, 0.0, 1.0),
};

static std::vector<uint32_t> icosahedronIndices = {
	0, 11, 5,
	0, 5, 1,
	0, 1, 7,
	0, 7, 10,
	0, 10, 11,
	1, 5, 9,
	5, 11, 4,
	11, 10, 2,
	10, 7, 6,
	7, 1, 8,
	3, 9, 4,
	3, 4, 2,
	3, 2, 6,
	3, 6, 8,
	3, 8, 9,
	4, 9, 5,
	2, 4, 11,
	6, 2, 10,
	8, 6, 7,
	9, 8, 1,
};

namespace Primitives
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <SphereAtt.h>
#include <JEnd.h>

#endif

	Sphere::Sphere(nlohmann::json& json) : Primitive(json)
	{
#include <Attributes/JInit.h>
#include <SphereAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <SphereAtt.h>
#include <JEnd.h>

		teselationPoints = icosahedronNNPoints;
		NormalizePoints(teselationPoints, 0.5f);
		teselationIndices = icosahedronIndices;
	}

#if defined(_EDITOR)
	void Sphere::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <SphereAtt.h>
#include <JEnd.h>
		j.erase("uuid");
	}
#endif

	void Sphere::PrepareMesh()
	{
		TeselateIcosahedron(teselationPoints, teselationIndices, teselationLevel());
	}

	std::vector<uint32_t> Sphere::GetIndices()
	{
		return teselationIndices;
	}

	void Sphere::NormalizePoint(DirectX::XMFLOAT3& p, float factor)
	{
		XMFLOAT4 vp = { p.x,p.y,p.z,0.0f };
		XMVECTOR vpf32 = XMLoadFloat4(&vp);
		XMVECTOR nvp = XMVector3Normalize(vpf32) * factor;
		p = *(XMFLOAT3*)nvp.m128_f32;
	}

	void Sphere::NormalizePoints(std::vector<XMFLOAT3>& points, float factor)
	{
		for (auto& p : points)
		{
			NormalizePoint(p, factor);
		}
	}

	void Sphere::TeselateIcosahedron(std::vector<XMFLOAT3>& points, std::vector<uint32_t>& indices, uint32_t teleselationCount)
	{
		while (teleselationCount != 0)
		{
			std::vector<uint32_t> newIndices;
			for (unsigned int i = 0U; i < indices.size();)
			{
				uint32_t i0 = indices.at(i);
				uint32_t i1 = indices.at(i + 1);
				uint32_t i2 = indices.at(i + 2);

				XMFLOAT3 v0 = points.at(i0);
				XMFLOAT3 v1 = points.at(i1);
				XMFLOAT3 v2 = points.at(i2);

				XMFLOAT3 v3 = { 0.5f * (v0.x + v1.x), 0.5f * (v0.y + v1.y),0.5f * (v0.z + v1.z) };
				XMFLOAT3 v4 = { 0.5f * (v1.x + v2.x), 0.5f * (v1.y + v2.y),0.5f * (v1.z + v2.z) };
				XMFLOAT3 v5 = { 0.5f * (v2.x + v0.x), 0.5f * (v2.y + v0.y),0.5f * (v2.z + v0.z) };

				NormalizePoint(v3, 0.5f);
				NormalizePoint(v4, 0.5f);
				NormalizePoint(v5, 0.5f);

				uint32_t i3 = static_cast<uint32_t>(points.size());
				uint32_t i4 = static_cast<uint32_t>(points.size() + 1ULL);
				uint32_t i5 = static_cast<uint32_t>(points.size() + 2ULL);

				points.push_back(v3);
				points.push_back(v4);
				points.push_back(v5);

				newIndices.push_back(i0); newIndices.push_back(i3); newIndices.push_back(i5);
				newIndices.push_back(i3); newIndices.push_back(i1); newIndices.push_back(i4);
				newIndices.push_back(i5); newIndices.push_back(i4); newIndices.push_back(i2);
				newIndices.push_back(i3); newIndices.push_back(i4); newIndices.push_back(i5);

				i += 3;
			}
			teleselationCount--;
			indices = newIndices;
		}
	}

	std::vector<Sphere::VertexType> Sphere::GetVertices()
	{
		std::vector<VertexType> vertices;
		for (auto& p : teselationPoints)
		{
			float u = std::atan2f(p.x, p.z) / (2.0f * static_cast<float>(M_PI)) + 0.5f;
			float v = std::asinf(p.y) / static_cast<float>(M_PI) + 0.5f;
			VertexType vertex = {
				.Position = p,
				.Normal = p,
				.TexCoord = XMFLOAT2(u,v)
			};
			vertices.push_back(vertex);
		}
		return vertices;
	}
};
