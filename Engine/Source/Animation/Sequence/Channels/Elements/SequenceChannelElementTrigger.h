#pragma once

#include <UUID.h>
#include "SequenceChannelElement.h"
#include <string>
#include <SimpleMath.h>
#include <nlohmann/json.hpp>

struct SequenceChannelElementTrigger : SequenceChannelElement
{
	SequenceChannelElementTrigger() :SequenceChannelElement() {}
	SequenceChannelElementTrigger(const nlohmann::json& j);
	bool operator==(const SequenceChannelElementTrigger& other) const;
	nlohmann::json json();

	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
	std::string bone;
	std::string onEnter;
	std::string onLeave;
	bool enabled;
#if defined(_EDITOR)
	RenderableID renderable;
	RenderableID renderableLines;
	JUUID physicObject;
#endif
};