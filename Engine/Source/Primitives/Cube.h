#pragma once

#include "Primitive.h"
#include <VertexFormats.h>

namespace Primitives
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <CubeAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <CubeAtt.h>
#include <JEnd.h>

#endif
	struct Cube : Primitive
	{
#include <Attributes/JFlags.h>
#include <CubeAtt.h>
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include <CubeAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <CubeAtt.h>
#include <JEnd.h>

		static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL_TEXCOORD0;
		typedef Vertex<VertexClass> VertexType;

		Cube(nlohmann::json& json);
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
		std::vector<uint32_t> GetIndices();
		std::vector<VertexType> GetVertices();
	};
};
