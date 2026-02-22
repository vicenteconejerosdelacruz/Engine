#ifndef _PRIMITIVE_CUBE_H
#define _PRIMITIVE_CUBE_H

#include "Primitive.h"
#include <VertexFormats.h>

namespace Primitives
{
	struct Cube : Primitive
	{
		static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL_TEXCOORD0;
		typedef Vertex<VertexClass> VertexType;

		Cube(nlohmann::json& json) : Primitive(json) {}
		std::vector<uint32_t> GetIndices();
		std::vector<VertexType> GetVertices();
	};
};

#endif