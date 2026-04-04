#include "pch.h"
#include "SequencePlayer.h"
#include <Scene.h>
#include <SceneObject.h>
#include <NoMath.h>

SequencePlayer::SequencePlayer()
{
	time = 0.0f;
	runningFrame = 0;
	currentFrame = 0;
	loop = false;
	newSequence = false;
}

SequencePlayer::SequencePlayer(const Sequence& seq, RenderableID renderable)
{
	sequence = seq;
	time = 0.0f;
	runningFrame = 0;
	currentFrame = 0;
	loop = false;
	newSequence = false;
	this->renderable = renderable;
}

void SequencePlayer::SetSequence(const Sequence& seq, RenderableID renderable)
{
	sequence = seq;
	time = 0.0f;
	runningFrame = 0;
	currentFrame = 0;
	loop = false;
	newSequence = true;
	runnedFrames.clear();
	this->renderable = renderable;
}

#if defined(_EDITOR)
void SequencePlayer::CreateSequenceTriggersAvatars(JUUID camera)
{
	using namespace Scene;

	auto triggers = sequence.GetTriggerElements();
	if (triggers.size() == 0ULL) return;

	nlohmann::json triggersJ = nlohmann::json::array({});
	SceneUnitId unit = renderable.unit();

	for (auto* t : triggers)
	{
		JUUID uuid = getUUID();
		t->triggerRenderable = MAKESUUUID(unit, uuid);
		nlohmann::json tj =
		{
			{
				"meshMaterial",
				{
					{ "material", GetMaterialUUIDByName("Floor")},
					{ "mesh",
						{
							{ "primitive", "f7786ac1-e296-4e9a-a7e6-6f1949de75ef" }
						}
					}
				}
			},
			{ "castShadows", false },
			{ "shadowed", false },
			{ "name" , uuid },
			{ "uuid" , uuid },
			{ "position", FromXMFLOAT3(t->position) },
			{ "topology", "TRIANGLELIST" },
			{ "rotation" , FromXMFLOAT3(t->rotation) },
			{ "scale" , FromXMFLOAT3(t->scale) },
			{ "skipMeshes" , {}},
			{ "visible" , true },
			{ "hidden" , true},
			{ "cameras", { camera }},
			{ "depthStencil",
				{
					{ "BackFace",
						{
							{ "StencilDepthFailOp", "KEEP"},
							{ "StencilFailOp", "KEEP"},
							{ "StencilFunc", "ALWAYS"},
							{ "StencilPassOp", "KEEP" }
						}
					},
					{ "DepthEnable", true },
					{ "DepthFunc", "LESS" },
					{ "DepthWriteMask", "ALL" },
					{ "FrontFace",
						{
							{ "StencilDepthFailOp", "KEEP"},
							{ "StencilFailOp", "KEEP"},
							{ "StencilFunc", "ALWAYS"},
							{ "StencilPassOp", "KEEP" }
						}
					},
					{ "StencilEnable", false},
					{ "StencilReadMask", 255},
					{ "StencilWriteMask", 255 }
				}
			}
		};

		triggersJ.push_back(tj);
	}

	nlohmann::json data = {
		{ "renderables", triggersJ }
	};

	AttachLevelIntoScene(unit, "triggers-avatars", data, [=](SceneUnitId) {});
}
void SequencePlayer::DestroySequenceTriggersAvatars()
{
	auto triggers = sequence.GetTriggerElements();
	if (triggers.size() == 0ULL) return;

	for (auto* t : triggers)
	{
		t->triggerRenderable->markedForDelete = true;
	}
}
#endif

void SequencePlayer::CreateSequenceTriggers()
{
	XMMATRIX world = renderable->world();
	Animation::BonesTransformations& bonesTransformation = renderable->bonesTransformation;

	auto triggers = sequence.GetTriggerElements();
	if (triggers.size() == 0ULL) return;

	nlohmann::json triggersJ = nlohmann::json::array({});
	SceneUnitId unit = renderable.unit();

	for (auto* t : triggers)
	{
		triggersJ.push_back(t->CreateTriggerJson(renderable, world, bonesTransformation));
		t->trigger = MAKESUUUID(0ULL, triggersJ.back().at("uuid"));
		t->triggerBuilt = std::make_unique<std::atomic_bool>(false);
	}

	nlohmann::json data = {
		{ "triggers", triggersJ }
	};

	AttachLevelIntoScene(unit, "triggers", data, [=](SceneUnitId)
		{
			for (auto* t : triggers)
			{
				t->trigger = MAKESUUUID(unit, std::get<1>(t->trigger()));
				t->triggerBuilt->store(true);
			}
		}
	);
}

void SequencePlayer::DestroySequenceTriggers()
{
	auto triggers = sequence.GetTriggerElements();
	for (auto* t : triggers)
	{
		t->triggerBuilt->store(false);
		t->triggerBuilt = nullptr;
		t->trigger->markedForDelete = true;
		t->trigger.clear();
	}
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

void SequencePlayer::ApplyFrameValues()
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

void SequencePlayer::ApplyFrameTriggerAvatarValues()
{
	auto triggers = sequence.GetTriggerElements();
	for (auto* t : triggers)
	{
		t->ApplyFrameTriggerAvatarValues(currentFrame, renderable);
	}
}

void SequencePlayer::ApplyFrameTriggerValues()
{
	auto triggers = sequence.GetTriggerElements();
	for (auto* t : triggers)
	{
		t->ApplyFrameTriggerValues(currentFrame, renderable);
	}
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
