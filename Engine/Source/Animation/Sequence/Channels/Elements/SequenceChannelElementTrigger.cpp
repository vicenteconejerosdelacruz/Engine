#include "pch.h"
#include "SequenceChannelElementTrigger.h"
#include <NoMath.h>
#include <Renderable/Renderable.h>
#include <Animated.h>

SequenceChannelElementTrigger::SequenceChannelElementTrigger(const nlohmann::json& j) :SequenceChannelElement(j)
{
	position = ToXMFLOAT3(j.at("position"));
	rotation = ToXMFLOAT3(j.at("rotation"));
	scale = ToXMFLOAT3(j.at("scale"));
	color = j.contains("color") ? ToXMFLOAT4(j.at("color")) : XMFLOAT4(227.0f / 255.0f, 119.0f / 255.0f, 19.0f / 255.0f, 0.3f);
	bone = j.at("bone");
	onEnter = j.at("onEnter");
	onLeave = j.at("onLeave");
	objectMask = j.at("objectMask");
	collisionMask = j.at("collisionMask");
	enabled = false;
}

bool SequenceChannelElementTrigger::operator==(const SequenceChannelElementTrigger& other) const
{
	return frameStart == other.frameStart
		&& frameEnd == other.frameEnd
		&& position.x == other.position.x
		&& position.y == other.position.y
		&& position.z == other.position.z
		&& rotation.x == other.rotation.x
		&& rotation.y == other.rotation.y
		&& rotation.z == other.rotation.z
		&& scale.x == other.scale.x
		&& scale.y == other.scale.y
		&& scale.z == other.scale.z
		&& color.x == other.color.x
		&& color.y == other.color.y
		&& color.z == other.color.z
		&& color.w == other.color.w
		&& bone == other.bone
		&& onEnter == other.onEnter
		&& onLeave == other.onLeave
		&& objectMask == other.objectMask
		&& collisionMask == other.collisionMask;
}

nlohmann::json SequenceChannelElementTrigger::json()
{
	nlohmann::json j =
	{
		{ "frameStart", frameStart},
		{ "frameEnd" , frameEnd},
		{ "position", FromXMFLOAT3(position) },
		{ "rotation", FromXMFLOAT3(rotation) },
		{ "scale", FromXMFLOAT3(scale) },
		{ "color", FromXMFLOAT4(color) },
		{ "bone", bone },
		{ "onEnter", onEnter },
		{ "onLeave", onLeave },
		{ "objectMask", objectMask },
		{ "collisionMask", collisionMask }
	};

	return j;
}

void SequenceChannelElementTrigger::ApplyFrameTriggerAvatarValues(int frame, RenderableID renderable)
{
	if (!triggerRenderable) return;

	XMMATRIX world = renderable->world();
	bool visible = frame >= frameStart && frame <= frameEnd;
	triggerRenderable->visible(visible);
	triggerLines->visible(visible);

	Animation::NodeTransformsMap& nodesTransformation = renderable->animable->animations->globalNodeTransforms;
	auto [fPos, fRot, tRotQ, fScl] = GetTransformation(world, nodesTransformation);
	triggerRenderable->position(fPos);
	triggerRenderable->rotation(fRot);
	triggerRenderable->rotationQ(tRotQ);
	triggerRenderable->scale(fScl);
	triggerLines->position(fPos);
	triggerLines->rotation(fRot);
	triggerLines->rotationQ(tRotQ);
	triggerLines->scale(fScl);
}

void SequenceChannelElementTrigger::ApplyFrameTriggerValues(int frame, RenderableID renderable)
{
	if (trigger.empty() || trigger.unit() == 0ULL || triggerBuilt == nullptr || triggerBuilt->load() == false) return;
	XMMATRIX world = renderable->world();
	Animation::NodeTransformsMap& nodesTransformation = renderable->animable->animations->globalNodeTransforms;
	auto [fPos, fRot, tRotQ, fScl] = GetTransformation(world, nodesTransformation);
	trigger->at("position") = FromXMFLOAT3(fPos);
	trigger->at("rotation") = FromXMFLOAT3(fRot);
	trigger->rotationQ(tRotQ);
}

std::tuple<XMFLOAT3, XMFLOAT3, XMVECTOR, XMFLOAT3> SequenceChannelElementTrigger::GetTransformation(XMMATRIX world, Animation::NodeTransformsMap& nodesTransformation)
{
	XMMATRIX nodeSpace = nodesTransformation.at(bone);
	XMMATRIX nodeWorldSpace = XMMatrixMultiply(XMMatrixTranspose(nodeSpace), world);
	XMVECTOR nodeWorldScale, nodeWorldRotationQ, nodeWorldTranslation;
	XMMatrixDecompose(&nodeWorldScale, &nodeWorldRotationQ, &nodeWorldTranslation, nodeWorldSpace);

	XMVECTOR tPos = XMVector3Transform(XMLoadFloat3(&position), nodeWorldSpace);
	XMVECTOR tScl = XMLoadFloat3(&scale);

	XMVECTOR tRotQ = XMQuaternionMultiply(nodeWorldRotationQ, XMQuaternionRotationRollPitchYaw(
		XMConvertToRadians(rotation.x),
		XMConvertToRadians(rotation.y),
		XMConvertToRadians(rotation.z))
	);

	XMFLOAT3 fPos, fRot, fScl;
	fRot = Quaternion2Euler(tRotQ); //we convert to euler without much purpose, but anyway
	XMStoreFloat3(&fPos, tPos);
	XMStoreFloat3(&fScl, XMVectorAbs(XMVectorMultiply(tScl, nodeWorldScale)));

	return std::make_tuple(fPos, fRot, tRotQ, fScl);
}

nlohmann::json SequenceChannelElementTrigger::CreateTriggerJson(RenderableID renderable, XMMATRIX world, Animation::NodeTransformsMap& nodesTransformation)
{
	auto [fPos, fRot, tRotQ, fScl] = GetTransformation(world, nodesTransformation);
	JUUID uuid = getUUID();
	auto bindings = renderable->GetScriptBindings();

	nlohmann::json j = {
		{ "behavior", PhysicsBehaviorToString.at(PB_Trigger) },
		{ "collisionMask", collisionMask },
		{ "geometry", GetPhysicGeometryUUIDByName("cube") },
		{ "hidden", true },
		{ "name", uuid },
		{ "position", FromXMFLOAT3(fPos) },
		{ "rotation", FromXMFLOAT3(fRot) },
		{ "rotationQ", FromXMVECTOR(tRotQ) },
		{ "scale", FromXMFLOAT3(fScl) },
		{ "overrideColor", true },
		{ "color", FromXMFLOAT4(color) },
		{ "objectMask", objectMask },
		{ "onEnter", onEnter },
		{ "onLeave", onLeave },
		{ "trigger", true },
		{ "uuid", uuid },
		{ "countEnter", 1 },
		{ "countLeave", 1 },
	};
	if (bindings.size() > 0ULL)
	{
		j["bindings"] = nlohmann::json::array({});
		for (auto& b : bindings)
		{
			j["bindings"].push_back(FromScriptBinding(b));
		}
	}
	return j;
}
