#pragma once
#include "SequenceChannelElement.h"

struct SequenceChannelElementScript : SequenceChannelElement
{
	SequenceChannelElementScript();
	SequenceChannelElementScript(const nlohmann::json& j);
	bool operator==(const SequenceChannelElementScript& other) const;
	nlohmann::json json();

	std::string script;
};