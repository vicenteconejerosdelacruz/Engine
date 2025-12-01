#include "pch.h"
#include "SequenceChannelElementScript.h"
#include <Scripting.h>

SequenceChannelElementScript::SequenceChannelElementScript() :SequenceChannelElement()
{
	script = "";
}

SequenceChannelElementScript::SequenceChannelElementScript(const nlohmann::json& j) :SequenceChannelElement(j)
{
	script = j.at("script");
}

bool SequenceChannelElementScript::operator==(const SequenceChannelElementScript& other) const {
	return frameStart == other.frameStart && frameEnd == other.frameEnd && script == other.script;
}

nlohmann::json SequenceChannelElementScript::json()
{
	nlohmann::json j = {
		{ "frameStart", frameStart },
		{ "frameEnd", frameEnd },
		{ "script", script },
	};
	return j;
}
