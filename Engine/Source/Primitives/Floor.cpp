#include "pch.h"
#include "Floor.h"

using namespace Primitives;

static std::vector<uint32_t> indices = {
	0, 1, 2, 3, 2, 1 //+Y
};

static std::vector<Floor::VertexType> vertices = {
	//+Y
	{ XMFLOAT3(-1.0f,  0.0f,  1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f) },
	{ XMFLOAT3(1.0f,  0.0f,  1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f) },
	{ XMFLOAT3(-1.0f,  0.0f, -1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f) },
	{ XMFLOAT3(1.0f,  0.0f, -1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f) }
};

namespace Primitives
{
	std::vector<uint32_t> Floor::GetIndices()
	{
		return indices;
	}

	std::vector<Floor::VertexType> Floor::GetVertices()
	{
		return vertices;
	}
};