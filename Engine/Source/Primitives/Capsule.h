#pragma once

#include "Primitive.h"
#include <VertexFormats.h>

namespace Primitives
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <CapsuleAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <CapsuleAtt.h>
#include <JEnd.h>

#endif

	struct Capsule : Primitive
	{
#include <Attributes/JFlags.h>
#include <CapsuleAtt.h>
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include <CapsuleAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <CapsuleAtt.h>
#include <JEnd.h>

		static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL_TEXCOORD0;
		typedef Vertex<VertexClass> VertexType;

		Capsule(nlohmann::json& json);
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
		virtual void PrepareMesh();

		std::vector<uint32_t> GetIndices();
		std::vector<VertexType> GetVertices();

		void NormalizePoint(DirectX::XMFLOAT3& p, float factor = 1.0f);
		void NormalizePoints(std::vector<XMFLOAT3>& points, float factor = 1.0f);
		void Teselate(std::vector<XMFLOAT3>& points, std::vector<uint32_t>& indices, uint32_t teselationLevel);

		std::vector<XMFLOAT3> teselationPoints;
		std::vector<uint32_t> teselationIndices;
	};
};
