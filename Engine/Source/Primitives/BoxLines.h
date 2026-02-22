#ifndef _PRIMITIVE_BOXLINES_H
#define _PRIMITIVE_BOXLINES_H

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

#endif