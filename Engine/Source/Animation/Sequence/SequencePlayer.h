#pragma once
#include "Sequence.h"

struct SequencePlayer
{
	Sequence* sequence;
	float time;
	int runningFrame;
	int currentFrame;
	bool loop;
	bool newSequence;
	std::set<int> runnedFrames;
	RenderableUUID renderable;

	SequencePlayer();
	SequencePlayer(Sequence* seq, JUUID uuid);
	void SetSequence(Sequence* seq, JUUID uuid);
	void Step(float dt);
	void SetTime(float t);
	void StepFrame(int df);
	void SetFrame(int frame, bool runningPlayer = true);
	void ApplyFrameValues(RenderableUUID renderable);
	void CreateFrameSoundFXs(int frame);
	void ExecuteFrameScripts(int frame);
	void ResetFrames();
};