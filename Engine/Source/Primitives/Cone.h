#pragma once
#include "Primitive.h"
#include <VertexFormats.h>

namespace Primitives
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <ConeAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <ConeAtt.h>
#include <JEnd.h>

#endif

	struct Cone : Primitive
	{
#include <Attributes/JFlags.h>
#include <ConeAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <ConeAtt.h>
#include <JEnd.h>

		static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL;
		typedef Vertex<VertexClass> VertexType;

		std::vector<VertexType> vertices;
		std::vector<uint32_t> indices;

		Cone(nlohmann::json& json);
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
		virtual void PrepareMesh();

		std::vector<uint32_t> GetIndices();
		std::vector<VertexType> GetVertices();
	};

	template<>
	inline void DrawPrimitiveAttributes<Cone>(nlohmann::json& json, std::function<void(nlohmann::json new_atts)> update)
	{
		Cone s(json);
		std::vector<JObject*> jvec = { &s };

		nlohmann::json source = nlohmann::json::parse(s.dump());

		auto drawers = GetConeDrawers();
		auto attributes = GetConeAttributes();
		for (auto& [att, _] : attributes)
		{
			drawers.at(att)(att, jvec);
		}

		nlohmann::json target = nlohmann::json::parse(s.dump());
		if (nlohmann::json::diff(source, target).size() > 0)
		{
			update(target);
		}
	}
};