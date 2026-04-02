#include "pch.h"
#include "Sequence.h"
#include <Sound/Sound.h>
#include <Scripting.h>

Sequence::Sequence()
{
	framesPerSecond = 60;
	totalFrames = 160;
}

Sequence::Sequence(nlohmann::json j)
{
	framesPerSecond = j.at("framesPerSecond");
	totalFrames = j.at("totalFrames");
	for (size_t i = 0ULL; i < j.at("sequenceChannels").size(); i++)
	{
		sequenceChannels.push_back(j.at("sequenceChannels").at(i));
	}
}

Sequence::Sequence(const Sequence& seq)
{
	framesPerSecond = seq.framesPerSecond;
	totalFrames = seq.totalFrames;
	sequenceChannels = seq.sequenceChannels;
}

nlohmann::json Sequence::json()
{
	nlohmann::json seqChannels = nlohmann::json::array();
	for (auto& seqChannel : sequenceChannels)
	{
		seqChannels.push_back(seqChannel.json());
	}
	nlohmann::json j(
		{
			{ "framesPerSecond", framesPerSecond },
			{ "totalFrames", totalFrames },
			{ "sequenceChannels", seqChannels }
		}
	);
	return j;
}

bool Sequence::operator==(const Sequence& other) const {

	if (framesPerSecond != other.framesPerSecond) return false;
	if (totalFrames != other.totalFrames) return false;

	return std::equal(sequenceChannels.begin(), sequenceChannels.end(), other.sequenceChannels.begin());
}

std::string Sequence::GetAnimationNameAtFrame(int frame)
{
	for (auto it = sequenceChannels.rbegin(); it != sequenceChannels.rend(); it++)
	{
		int index = it->GetFirstElementIndexBetweenFrames(frame, frame);
		if (index == -1) continue;
		auto& element = it->elements.at(index);
		if (element.type != SCET_Animation) continue;
		return element.animation.animation;
	}
	return "";
}

SequenceChannelElementAnimation* Sequence::GetAnimationElementAtFrame(int frame)
{
	std::vector<SequenceChannelElementAnimation*> animations;
	for (SequenceChannel& channel : sequenceChannels)
	{
		SequenceChannelElementAnimation* anim = channel.GetAnimationElementAtFrame(frame);
		if (anim == nullptr) continue;
		animations.push_back(anim);
	}

	if (animations.size() == 0ULL) return nullptr;

	std::sort(animations.begin(), animations.end(), [](SequenceChannelElementAnimation* animA, SequenceChannelElementAnimation* animB)
		{
			int endA = animA->frameEnd;
			int endB = animB->frameEnd;
			return endA > endB;
		}
	);

	return *animations.begin();
}

XMMATRIX Sequence::GetTransformationAtFrame(int frame)
{
	std::vector<SequenceChannelElementTransformation*> transformations;
	for (SequenceChannel& channel : sequenceChannels)
	{
		SequenceChannelElementTransformation* t = channel.GetTransformationElementAtFrame(frame);
		if (t == nullptr) continue;
		transformations.push_back(t);
	}

	if (transformations.size() == 0ULL) return XMMatrixIdentity();

	std::sort(transformations.begin(), transformations.end(), [](SequenceChannelElementTransformation* tA, SequenceChannelElementTransformation* tB)
		{
			int endA = tA->frameEnd;
			int endB = tB->frameEnd;
			return endA > endB;
		}
	);

	SequenceChannelElementTransformation* transformation = *transformations.begin();

	return transformation->GetTransformationInFrame(frame);
}

void Sequence::CreateSoundFXsAtFrame(int frame, SceneUnitId id)
{
	std::set<SequenceChannelElementSoundFX*> soundfxs;
	for (SequenceChannel& channel : sequenceChannels)
	{
		SequenceChannelElementSoundFX* sfx = channel.GetSoundFXToCreateAtFrame(frame);
		if (sfx == nullptr) continue;
		soundfxs.insert(sfx);
	}

	if (soundfxs.size() == 0ULL) return;

	nlohmann::json sounds = { {"sounds",{}} };

	std::set<JUUID> soundsUUID;
	for (auto& sfx : soundfxs)
	{
		SoundJsonID sjson = sfx->sound();
		JUUID soundFXUUID = getUUID();
		soundsUUID.insert(soundFXUUID);
		nlohmann::json jsound =
		{
			{ "uuid", soundFXUUID },
			{ "name", sjson->name() },
			{ "sound", sfx->sound() },
			{ "autoPlay", true }
		};

		sounds.at("sounds").push_back(jsound);
	}

	AttachLevelIntoScene(id, "sfx", sounds, [&](SceneUnitId id) {});
}

void Sequence::RunScriptAtFrame(int frame, RenderableID renderable)
{
	using namespace Scripting;
	for (SequenceChannel& channel : sequenceChannels)
	{
		SequenceChannelElementScript* script = channel.GetScriptToRunAtFrame(frame);
		if (script == nullptr) continue;
		RunScript(script->script, renderable());
	}
}

bool Sequence::Runnable() const
{
	return !sequenceChannels.empty();
}

SequenceChannelElementType Sequence::GetElementType(unsigned int channelId, unsigned int frame)
{
	auto& channel = sequenceChannels.at(channelId);
	int elemIdx = channel.GetFirstElementIndexBetweenFrames(frame, frame);
	return channel.elements.at(elemIdx).type;
}
