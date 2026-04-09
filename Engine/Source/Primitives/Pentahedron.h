#pragma once

#include "Primitive.h"
#include <VertexFormats.h>

namespace Primitives
{
	struct Pentahedron : Primitive
	{
		static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL_TANGENT_TEXCOORD0;
		typedef Vertex<VertexClass> VertexType;

		Pentahedron(nlohmann::json& json) : Primitive(json) {}
		std::vector<uint32_t> GetIndices();
		std::vector<VertexType> GetVertices();
	};
};
