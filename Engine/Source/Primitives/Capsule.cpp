#include "pch.h"
#include "Capsule.h"
#include <corecrt_math_defines.h>

const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

//non normalized points, don't use directly
static std::vector<XMFLOAT3> capsuleBasePoints = {
	XMFLOAT3(0.0f,1.0f,0.0f),
	XMFLOAT3(0.0f,0.0f,1.0f),
	XMFLOAT3(1.0f,0.0f,0.0f),
	XMFLOAT3(0.0f,0.0f,-1.0f),
	XMFLOAT3(-1.0f,0.0f,0.0f),
};

static std::vector<uint32_t> capsuleBaseIndices = {
	0, 1, 2,
	0, 2, 3,
	0, 3, 4,
	0, 4, 1
};

namespace Primitives
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <CapsuleAtt.h>
#include <JEnd.h>

#endif

	Capsule::Capsule(nlohmann::json& json) : Primitive(json)
	{
#include <Attributes/JInit.h>
#include <CapsuleAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <CapsuleAtt.h>
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include <CapsuleAtt.h>
#include <JEnd.h>

		teselationPoints = capsuleBasePoints;
		NormalizePoints(teselationPoints, 0.5f);
		teselationIndices = capsuleBaseIndices;
	}

#if defined(_EDITOR)
	void Capsule::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <CapsuleAtt.h>
#include <JEnd.h>
		j.erase("uuid");
	}
#endif

	void Capsule::PrepareMesh()
	{
		Teselate(teselationPoints, teselationIndices, teselationLevel());
	}

	std::vector<uint32_t> Capsule::GetIndices()
	{
		return teselationIndices;
	}

	void Capsule::NormalizePoint(DirectX::XMFLOAT3& p, float factor)
	{
		XMFLOAT4 vp = { p.x,p.y,p.z,0.0f };
		XMVECTOR vpf32 = XMLoadFloat4(&vp);
		XMVECTOR nvp = XMVector3Normalize(vpf32) * factor;
		p = *(XMFLOAT3*)nvp.m128_f32;
	}

	void Capsule::NormalizePoints(std::vector<XMFLOAT3>& points, float factor)
	{
		for (auto& p : points)
		{
			NormalizePoint(p, factor);
		}
	}

	void Capsule::Teselate(std::vector<XMFLOAT3>& points, std::vector<uint32_t>& indices, uint32_t teselationCount)
	{
		//do the teselation of the triangle soup
		while (teselationCount != 0)
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
			teselationCount--;
			indices = newIndices;
		}

		//gather the faces that are touching the bottom of the mesh(2 vertices with y=0.0f)
		std::set<uint32_t> bottomFaces;
		for (unsigned int i = 0U; i < indices.size();)
		{
			uint32_t i0 = indices.at(i);
			uint32_t i1 = indices.at(i + 1);
			uint32_t i2 = indices.at(i + 2);

			XMFLOAT3 v0 = points.at(i0);
			XMFLOAT3 v1 = points.at(i1);
			XMFLOAT3 v2 = points.at(i2);

			if ((v0.y == 0.0f && v1.y == 0.0f) || (v1.y == 0.0f && v2.y == 0.0f) || (v0.y == 0.0f && v2.y == 0.0f))
			{
				bottomFaces.insert(i / 3);
			}

			i += 3;
		}

		//flip the y coord
		unsigned int nIndices = static_cast<unsigned int>(indices.size());
		unsigned int nPoints = static_cast<unsigned int>(points.size());
		std::vector<XMFLOAT3> bottomPoints;
		std::transform(points.begin(), points.end(), std::back_inserter(bottomPoints), [](XMFLOAT3 p)
			{
				p.y *= -1.0f;
				return p;
			}
		);
		//attach the new points and indices
		points.insert(points.end(), bottomPoints.begin(), bottomPoints.end());
		for (unsigned int i = 0; i < nIndices; i++)
		{
			indices.push_back(indices.at(i) + nPoints);
		}

		auto getBottomIndices = [&](unsigned int face)
			{
				uint32_t i0 = indices.at(face * 3);
				uint32_t i1 = indices.at(face * 3 + 1);
				uint32_t i2 = indices.at(face * 3 + 2);

				XMFLOAT3 v0 = points.at(i0);
				XMFLOAT3 v1 = points.at(i1);
				XMFLOAT3 v2 = points.at(i2);

				if (v0.y == 0.0f && v1.y == 0.0f)
				{
					return std::make_tuple(i0, i1);
				}
				else if (v1.y == 0.0f && v2.y == 0.0f)
				{
					return std::make_tuple(i1, i2);
				}
				else if (v0.y == 0.0f && v2.y == 0.0f)
				{
					return std::make_tuple(i0, i2);
				}
				assert(!!!"bad face");
				return std::make_tuple(0U, 0U);
			};

		//swap the bottom indices to avoid backface culling issues
		for (unsigned int i = 0; i < nIndices / 3; i++)
		{
			unsigned int offset = i * 3 + nIndices;
			std::swap(indices[offset], indices[offset + 2]);
		}

		//insert the cilinder indices
		for (auto face : bottomFaces)
		{
			auto [i0, i1] = getBottomIndices(face);

			indices.insert(indices.end(), { i1, i0, i1 + nPoints });
			indices.insert(indices.end(), { i0 + nPoints, i1 + nPoints, i0 });
		}

		//scale by radius and it's done
		for (auto& p : points)
		{
			XMVECTOR sp = XMVectorScale(XMVector3Normalize(XMLoadFloat3(&p)), radius());
			XMStoreFloat3(&p, sp);
		}

		//apply the halfheight to all vertices
		for (unsigned int i = 0; i < nPoints; i++)
		{
			points.at(i).y += halfHeight();
			points.at(i + nPoints).y -= halfHeight();
		}

	}

	std::vector<Capsule::VertexType> Capsule::GetVertices()
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
