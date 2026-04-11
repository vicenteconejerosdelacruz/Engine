#pragma once

#include "SequenceChannelElement.h"
#include <unordered_map>
#include <string>
#include <SimpleMath.h>
#include <nlohmann/json.hpp>
#include "TransformationKeyFrame.h"
#include "SequenceChannelElementTransformation.h"

struct SequenceChannelElementBoneTransformation : public SequenceChannelElementTransformation
{
	SequenceChannelElementBoneTransformation() :SequenceChannelElementTransformation() {}
	SequenceChannelElementBoneTransformation(const nlohmann::json& j);
	bool operator==(const SequenceChannelElementBoneTransformation& other) const;
	nlohmann::json json() override;
	std::string GetBone() const;

	std::string bone;
};
