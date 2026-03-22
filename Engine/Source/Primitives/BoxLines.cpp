#include "pch.h"
#include "BoxLines.h"

using namespace Primitives;

static std::vector<uint32_t> indices =
{
	0, 1, 1, 3, 3, 2, 2, 0, // top
	4, 5, 5, 7, 7, 6, 6, 4, // bottom
	0, 4, 2, 6, 3, 7, 1, 5  // side lines
};

static std::vector<BoxLines::VertexType> vertices = {
	{ XMFLOAT3(-1.0f, +1.0f, -1.0f) }, // -X +Y -Z
	{ XMFLOAT3(+1.0f, +1.0f, -1.0f) }, // +X +Y -Z
	{ XMFLOAT3(-1.0f, +1.0f, +1.0f) }, // -X +Y +Z
	{ XMFLOAT3(+1.0f, +1.0f, +1.0f) }, // +X +Y +Z
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f) }, // -X -Y -Z
	{ XMFLOAT3(+1.0f, -1.0f, -1.0f) }, // +X -Y -Z
	{ XMFLOAT3(-1.0f, -1.0f, +1.0f) }, // -X -Y +Z
	{ XMFLOAT3(+1.0f, -1.0f, +1.0f) }  // +X -Y +Z
};

namespace Primitives
{
	std::vector<uint32_t> BoxLines::GetIndices()
	{
		return indices;
	}

	std::vector<BoxLines::VertexType> BoxLines::GetVertices()
	{
		return vertices;
	}
};