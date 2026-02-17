#pragma once
#include "Primitive.h"
#include <VertexFormats.h>

namespace Primitives
{
	struct Cone : Primitive
	{
		unsigned int spread = 36;
		static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL;
		typedef Vertex<VertexClass> VertexType;

		std::vector<VertexType> vertices;
		std::vector<uint32_t> indices;

		std::vector<uint32_t> GetIndices();
		std::vector<VertexType> GetVertices();

		Cone(nlohmann::json& json);
		//Cone(void* params);
		//Cone() :Cone(nullptr) {}
	};
};