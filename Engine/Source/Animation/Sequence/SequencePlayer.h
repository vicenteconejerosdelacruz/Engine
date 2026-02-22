#ifndef _SEQUENCE_PLAYER_H
#define _SEQUENCE_PLAYER_H

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
	void Step(float dt);
	void SetTime(float t);
	void StepFrame(int df);
	void SetFrame(int frame, bool runningPlayer = true);
	void ApplyFrameValues(RenderableID renderable);
	void CreateFrameSoundFXs(int frame);
	void ExecuteFrameScripts(int frame);
	void ResetFrames();
};
#endif