#include "pch.h"
#include "SequenceChannelElementBoneTransformation.h"

SequenceChannelElementBoneTransformation::SequenceChannelElementBoneTransformation(const nlohmann::json& j) : SequenceChannelElementTransformation(j)
{
	bone = j.contains("bone") ? j.at("bone") : "";
}

bool SequenceChannelElementBoneTransformation::operator==(const SequenceChannelElementBoneTransformation& other) const
{
	return bone == other.bone && keyFrames == other.keyFrames;
}

nlohmann::json SequenceChannelElementBoneTransformation::json()
{
	nlohmann::json j = SequenceChannelElementTransformation::json();
	j["bone"] = bone;
	return j;
}

std::string SequenceChannelElementBoneTransformation::GetBone() const
{
	return bone;
}
