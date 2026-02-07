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
	RenderableSUUUID renderable;

	SequencePlayer();
	SequencePlayer(const Sequence& seq, SceneUnitId id, JUUID uuid);
	void SetSequence(const Sequence& seq, SceneUnitId id, JUUID uuid);
	void Step(float dt);
	void SetTime(float t);
	void StepFrame(int df);
	void SetFrame(int frame, bool runningPlayer = true);
	void ApplyFrameValues(RenderableSUUUID renderable);
	void CreateFrameSoundFXs(int frame);
	void ExecuteFrameScripts(int frame);
	void ResetFrames();
};