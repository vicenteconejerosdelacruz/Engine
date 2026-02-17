#pragma once
#include <UUID.h>
#include <JObject.h>

namespace Primitives
{
	struct Primitive : JObject
	{
		Primitive(nlohmann::json& json) : JObject(json) {}
		//Primitive() { Primitive(nullptr); }
		//Primitive(void* params) {};
	};
}