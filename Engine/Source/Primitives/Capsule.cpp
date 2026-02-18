#include "pch.h"
#include "Capsule.h"
#include <corecrt_math_defines.h>

namespace Primitives
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <CapsuleAtt.h>
#include <JEnd.h>

#endif

	Capsule::Capsule(nlohmann::json& json) : Primitive(json)
	{
#include <Attributes/JInit.h>
#include <CapsuleAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <CapsuleAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void Capsule::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <CapsuleAtt.h>
#include <JEnd.h>
		j.erase("uuid");
	}
#endif

	void Capsule::PrepareMesh()
	{
	}

	std::vector<uint32_t> Capsule::GetIndices()
	{
		return std::vector<uint32_t>();
	}

	std::vector<Capsule::VertexType> Capsule::GetVertices()
	{
		return std::vector<Capsule::VertexType>();
	}
};
