#pragma once
#include "Primitive.h"
#include <VertexFormats.h>

namespace Primitives
{
	struct UtahTeapot : Primitive
	{
		static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL;
		typedef Vertex<VertexClass> VertexType;

		UtahTeapot(nlohmann::json& json) : Primitive(json) {}
		std::vector<uint32_t> GetIndices();
		std::vector<VertexType> GetVertices();
	};
};
