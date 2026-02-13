#pragma once
#include "Primitive.h"
#include <VertexFormats.h>

struct Sphere : public Primitive
{
	unsigned int teleselationLevel = 4U;
	static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL_TEXCOORD0;
	typedef Vertex<VertexClass> VertexType;

	std::vector<XMFLOAT3> teselationPoints;
	std::vector<uint32_t> teselationIndices;

	std::vector<uint32_t> GetIndices();
	std::vector<VertexType> GetVertices();

	Sphere(void* params);
	void NormalizePoint(DirectX::XMFLOAT3& p, float factor = 1.0f);
	void NormalizePoints(std::vector<XMFLOAT3>& points, float factor = 1.0f);
	void TeselateIcosahedron(std::vector<XMFLOAT3>& points, std::vector<uint32_t>& indices, uint32_t teleselationLevel);
};
