#pragma once

#include "Sequence.h"

struct SequencePlayer
{
	Sequence sequence;
	float time;
	int runningFrame;
	int currentFrame;
	bool loop;
	bool newSequence;
	std::set<int> runnedFrames;
	RenderableID renderable;

	SequencePlayer();
	SequencePlayer(const Sequence& seq, RenderableID renderable);
	void SetSequence(const Sequence& seq, RenderableID renderable);
#if defined(_EDITOR)
	void CreateSequenceTriggersAvatars(JUUID camera);
	void DestroySequenceTriggersAvatars();
#endif
	void CreateSequenceTriggers();
	void DestroySequenceTriggers();
	void Step(float dt);
	void SetTime(float t);
	void StepFrame(int df);
	void SetFrame(int frame, bool runningPlayer = true);
	void ApplyFrameValues();
#if defined(_EDITOR)
	void ApplyFrameTriggerAvatarValues();
#endif
	void ApplyFrameTriggerValues();
	void CreateFrameSoundFXs(int frame);
	void ExecuteFrameScripts(int frame);
	void ResetFrames();
};
