#pragma once

#include <UUID.h>
#include "SequenceChannelElement.h"
#include <string>
#include <SimpleMath.h>
#include <nlohmann/json.hpp>

namespace Scene
{
	DEF_SCENEOBJECT_ID_DEP(Trigger);
};
using namespace Scene;

struct SequenceChannelElementTrigger : SequenceChannelElement
{
	SequenceChannelElementTrigger() :SequenceChannelElement() {}
	SequenceChannelElementTrigger(const nlohmann::json& j);
	SequenceChannelElementTrigger& operator=(const SequenceChannelElementTrigger& other)
	{
		if (this != &other) {
			frameStart = other.frameStart;
			frameEnd = other.frameEnd;
			position = other.position;
			rotation = other.rotation;
			scale = other.scale;
			color = other.color;
			bone = other.bone;
			onEnter = other.onEnter;
			onLeave = other.onLeave;
			objectMask = other.objectMask;
			collisionMask = other.collisionMask;
		}
		return *this;
	}
	bool operator==(const SequenceChannelElementTrigger& other) const;
	nlohmann::json json();
	void ApplyFrameTriggerAvatarValues(int frame, RenderableID renderable);
	void ApplyFrameTriggerValues(int frame, RenderableID renderable);
	std::tuple<XMFLOAT3, XMFLOAT3, XMVECTOR, XMFLOAT3> GetTransformation(XMMATRIX world, Animation::NodeTransformsMap& nodesTransformation);
	nlohmann::json CreateTriggerJson(RenderableID renderable, XMMATRIX world, Animation::NodeTransformsMap& nodesTransformation);

	//attributes
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
	XMFLOAT4 color;
	std::string bone;
	std::string onEnter;
	std::string onLeave;
	unsigned int objectMask;
	unsigned int collisionMask;

	//instancing
	bool enabled;
#if defined(_EDITOR)
	RenderableID triggerRenderable;
	RenderableID triggerLines;
#endif
	TriggerID trigger;
	std::unique_ptr<std::atomic_bool> triggerBuilt;
};