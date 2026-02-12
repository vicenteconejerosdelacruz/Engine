#pragma once
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
#include <SimpleMath.h>
#include "Sequence.h"

struct AnimationSequences
{
	AnimationSequences() {}
	AnimationSequences(nlohmann::json j);
	nlohmann::json json();

	std::unordered_map<std::string, Sequence> sequences;
};

inline static AnimationSequences ToAnimationSequences(nlohmann::json j)
{
	AnimationSequences seq(j);
	return seq;
}

inline static nlohmann::json FromAnimationSequences(AnimationSequences s)
{
	return s.json();
}
