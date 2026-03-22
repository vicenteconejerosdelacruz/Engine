#ifndef _SEQUENCE_CHANNEL_ELEMENT_SCRIPT_H
#define _SEQUENCE_CHANNEL_ELEMENT_SCRIPT_H

#include "SequenceChannelElement.h"

struct SequenceChannelElementScript : SequenceChannelElement
{
	SequenceChannelElementScript();
	SequenceChannelElementScript(const nlohmann::json& j);
	bool operator==(const SequenceChannelElementScript& other) const;
	nlohmann::json json();

	std::string script;
};
#endif