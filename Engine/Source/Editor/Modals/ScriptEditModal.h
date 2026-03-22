#pragma once
#include <JObject.h>
#include "SimpleModal.h"

struct ScriptEditModal : SimpleModal
{
	void Init(JObject* j, std::string att);
	void Draw();

	JObject* json;
	std::string attribute;
	std::string script;
};