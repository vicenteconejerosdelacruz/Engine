#include "pch.h"

#include "AnimationSequences.h"
#include <NoMath.h>
#include <NoStd.h>
#include <SimpleMath.h>


AnimationSequences::AnimationSequences(nlohmann::json j)
{
	for (nlohmann::json::iterator it = j.begin(); it != j.end(); it++)
	{
		sequences[it.key()] = Sequence(it.value());
	}
}

nlohmann::json AnimationSequences::json()
{
	nlohmann::json j;
	for (auto& [name, sequence] : sequences)
	{
		j[name] = sequence.json();
	}
	return j;
}
