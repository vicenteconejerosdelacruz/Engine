#pragma once
#include "Primitive.h"
#include <VertexFormats.h>

namespace Primitives
{
	struct Decal : Primitive
	{
		static constexpr VertexClass VertexClass = VertexClass::POS_TEXCOORD0;
		typedef Vertex<VertexClass> VertexType;

		Decal(nlohmann::json& json) : Primitive(json) {}
		std::vector<uint32_t> GetIndices();
		std::vector<VertexType> GetVertices();
	};
};
