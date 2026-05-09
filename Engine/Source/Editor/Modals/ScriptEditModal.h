#pragma once
#include <JObject.h>
#include "SimpleModal.h"

struct ScriptEditModal : SimpleModal
{
	enum ScriptEditModalMode
	{
		Mode_JObject,
		Mode_Callback
	};
	void Init(JObject* j, std::string att);
	void Init(std::string att, std::string script, std::function<void(std::string)> writer);
	void Draw();

	ScriptEditModalMode mode;
	JObject* json;
	std::string attribute;
	std::string script;
	std::function<void(std::string)> write = nullptr;
};