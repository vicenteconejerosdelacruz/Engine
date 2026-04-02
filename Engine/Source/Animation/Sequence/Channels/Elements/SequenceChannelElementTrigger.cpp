#include "pch.h"
#include "SequenceChannelElementTrigger.h"
#include <NoMath.h>

SequenceChannelElementTrigger::SequenceChannelElementTrigger(const nlohmann::json& j) :SequenceChannelElement(j)
{
	position = ToXMFLOAT3(j.at("position"));
	rotation = ToXMFLOAT3(j.at("rotation"));
	scale = ToXMFLOAT3(j.at("scale"));
	bone = j.at("bone");
	onEnter = j.at("onEnter");
	onLeave = j.at("onLeave");
	enabled = false;
}

bool SequenceChannelElementTrigger::operator==(const SequenceChannelElementTrigger& other) const
{
	return frameStart == other.frameStart
		&& frameEnd == other.frameEnd
		&& position.x == other.position.x
		&& position.y == other.position.y
		&& position.z == other.position.z
		&& rotation.x == other.rotation.x
		&& rotation.y == other.rotation.y
		&& rotation.z == other.rotation.z
		&& scale.x == other.scale.x
		&& scale.y == other.scale.y
		&& scale.z == other.scale.z
		&& bone == other.bone
		&& onEnter == other.onEnter
		&& onLeave == other.onLeave;
}

nlohmann::json SequenceChannelElementTrigger::json()
{
	nlohmann::json j =
	{
		{ "frameStart", frameStart},
		{ "frameEnd" , frameEnd},
		{ "position", FromXMFLOAT3(position) },
		{ "rotation", FromXMFLOAT3(rotation) },
		{ "scale", FromXMFLOAT3(scale) },
		{ "bone", bone },
		{ "onEnter", onEnter },
		{ "onLeave", onLeave },
	};

	return j;
}