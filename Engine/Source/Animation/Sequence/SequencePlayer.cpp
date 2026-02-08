#include "pch.h"
#include "SequencePlayer.h"
#include <SceneObject.h>

SequencePlayer::SequencePlayer()
{
	time = 0.0f;
	runningFrame = 0;
	currentFrame = 0;
	loop = false;
	newSequence = false;
}

SequencePlayer::SequencePlayer(const Sequence& seq, SceneUnitId id, JUUID uuid)
{
	sequence = seq;
	time = 0.0f;
	runningFrame = 0;
	currentFrame = 0;
	loop = false;
	newSequence = false;
	renderable = MAKESUUUID(id, uuid);
}

void SequencePlayer::SetSequence(const Sequence& seq, SceneUnitId id, JUUID uuid)
{
	sequence = seq;
	time = 0.0f;
	runningFrame = 0;
	currentFrame = 0;
	loop = false;
	newSequence = true;
	runnedFrames.clear();
	renderable = MAKESUUUID(id, uuid);
}

void SequencePlayer::Step(float dt)
{
	if (!sequence.Runnable()) return;
	newSequence = false;

	float totalTimeMs = 1000.0f * static_cast<float>(sequence.totalFrames) / static_cast<float>(sequence.framesPerSecond);
	time += dt;
	if (time >= totalTimeMs)
	{
		time = (loop) ? fmodf(time, totalTimeMs) : totalTimeMs;
		currentFrame = (loop) ? static_cast<int>(static_cast<float>(sequence.framesPerSecond) * time / 1000.0f) : sequence.totalFrames;
		if (loop)
		{
			runnedFrames.clear();
		}
	}
	else
	{
		currentFrame = static_cast<int>(static_cast<float>(sequence.framesPerSecond) * time / 1000.0f);
	}

	int targetFrame = currentFrame;
	if (runningFrame < sequence.totalFrames && currentFrame == 0 && newSequence)
		targetFrame = sequence.totalFrames;

	while (runningFrame <= targetFrame && !newSequence)
	{
		if (!runnedFrames.contains(runningFrame))
		{
			CreateFrameSoundFXs(runningFrame);
			ExecuteFrameScripts(runningFrame);
		}
		runnedFrames.insert(runningFrame);
		runningFrame++;
		if (runningFrame > sequence.totalFrames)
		{
			if (loop)
			{
				runningFrame = 0;
				runnedFrames.clear();
			}
			else
			{
				runningFrame = sequence.totalFrames;
			}
			break;
		}
	}
}

void SequencePlayer::SetTime(float t)
{
	if (!sequence.Runnable()) return;
	newSequence = false;

	float totalTimeMs = 1000.0f * static_cast<float>(sequence.totalFrames) / static_cast<float>(sequence.framesPerSecond);
	time = t;
	if (time >= totalTimeMs)
	{
		time = (loop) ? fmodf(time, totalTimeMs) : totalTimeMs;
		currentFrame = (loop) ? static_cast<int>(static_cast<float>(sequence.framesPerSecond) * time / 1000.0f) : sequence.totalFrames;
		if (loop)
		{
			runnedFrames.clear();
		}
	}
	else
	{
		currentFrame = static_cast<int>(static_cast<float>(sequence.framesPerSecond) * time / 1000.0f);
	}

	int targetFrame = currentFrame;
	if (runningFrame < sequence.totalFrames && currentFrame == 0)
		targetFrame = sequence.totalFrames;

	while (runningFrame <= targetFrame && !newSequence)
	{
		if (!runnedFrames.contains(runningFrame))
		{
			CreateFrameSoundFXs(runningFrame);
			ExecuteFrameScripts(runningFrame);
		}
		runnedFrames.insert(runningFrame);
		runningFrame++;
		if (runningFrame > sequence.totalFrames)
		{
			if (loop)
			{
				runningFrame = 0;
				runnedFrames.clear();
			}
			else
			{
				runningFrame = sequence.totalFrames;
			}
			break;
		}
	}
}

void SequencePlayer::StepFrame(int df)
{
	if (!sequence.Runnable()) return;
	newSequence = false;

	currentFrame += df;
	if (currentFrame > sequence.totalFrames)
	{
		currentFrame = (loop) ? (currentFrame % sequence.totalFrames) : sequence.totalFrames;
		if (loop)
		{
			runnedFrames.clear();
		}
	}
	time = 1000.0f * static_cast<float>(currentFrame) / static_cast<float>(sequence.framesPerSecond);

	int targetFrame = currentFrame;
	if (runningFrame < sequence.totalFrames && currentFrame == 0)
		targetFrame = sequence.totalFrames;

	while (runningFrame <= targetFrame && !newSequence)
	{
		if (!runnedFrames.contains(runningFrame))
		{
			CreateFrameSoundFXs(runningFrame);
			ExecuteFrameScripts(runningFrame);
		}
		runnedFrames.insert(runningFrame);
		runningFrame++;
		if (runningFrame > sequence.totalFrames)
		{
			if (loop)
			{
				runningFrame = 0;
				runnedFrames.clear();
			}
			else
			{
				runningFrame = sequence.totalFrames;
			}
			break;
		}
	}
}

void SequencePlayer::SetFrame(int frame, bool runningPlayer)
{
	if (!sequence.Runnable()) return;
	newSequence = false;

	currentFrame = frame;
	if (currentFrame > sequence.totalFrames)
	{
		currentFrame = (loop) ? (currentFrame % sequence.totalFrames) : sequence.totalFrames;
		if (loop)
		{
			runnedFrames.clear();
		}
	}
	time = 1000.0f * static_cast<float>(currentFrame) / static_cast<float>(sequence.framesPerSecond);

	if (runningPlayer == false) return;

	int targetFrame = currentFrame;
	if (runningFrame < sequence.totalFrames && currentFrame == 0)
		targetFrame = sequence.totalFrames;

	while (runningFrame <= targetFrame && !newSequence)
	{
		if (!runnedFrames.contains(runningFrame))
		{
			CreateFrameSoundFXs(runningFrame);
			ExecuteFrameScripts(runningFrame);
		}
		runnedFrames.insert(runningFrame);
		runningFrame++;
		if (runningFrame > sequence.totalFrames)
		{
			if (loop)
			{
				runningFrame = 0;
				runnedFrames.clear();
			}
			else
			{
				runningFrame = sequence.totalFrames;
			}
			break;
		}
	}
}

void SequencePlayer::ApplyFrameValues(RenderableID renderable)
{
	SequenceChannelElementAnimation* animation = sequence.GetAnimationElementAtFrame(currentFrame);
	if (animation == nullptr)
	{
		renderable->animation("");
		renderable->animationFrame(0);
		renderable->animationTime(0.0f);
	}
	else
	{
		renderable->animation(animation->animation);
		renderable->animationFrame(currentFrame);
		renderable->animationTime(animation->GetTimeAtFrame(currentFrame));
	}

	renderable->animationTransformation = sequence.GetTransformationAtFrame(currentFrame);
}

void SequencePlayer::CreateFrameSoundFXs(int frame)
{
	sequence.CreateSoundFXsAtFrame(frame, renderable.unit());
}

void SequencePlayer::ExecuteFrameScripts(int frame)
{
	sequence.RunScriptAtFrame(frame, renderable);
}

void SequencePlayer::ResetFrames()
{
	runnedFrames.clear();
	runningFrame = 0;
	currentFrame = 0;
	time = 0.0f;
}
