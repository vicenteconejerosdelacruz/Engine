#pragma once
#include "Primitive.h"
#include <VertexFormats.h>

namespace Primitives
{
	struct BoxLines : Primitive
	{
		static constexpr VertexClass VertexClass = VertexClass::POS;
		typedef Vertex<VertexClass> VertexType;

		BoxLines(nlohmann::json& json) : Primitive(json) {}
		std::vector<uint32_t> GetIndices();
		std::vector<VertexType> GetVertices();
	};
};