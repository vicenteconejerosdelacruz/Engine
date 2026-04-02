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
	bone = j.at("bone");
	onEnter = j.at("onEnter");
	onLeave = j.at("onLeave");
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
		&& bone == other.bone
		&& onEnter == other.onEnter
		&& onLeave == other.onLeave;
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
		{ "bone", bone },
		{ "onEnter", onEnter },
		{ "onLeave", onLeave },
	};

	return j;
}

void SequenceChannelElementTrigger::ApplyFrameTriggerAvatarValues(int frame, RenderableID renderable)
{
	XMMATRIX world = renderable->world();
	Animation::BonesTransformations& bonesTransformation = renderable->bonesTransformation;
	bool visible = frame >= frameStart && frame <= frameEnd;
	triggerRenderable->visible(visible);
	if (bone.empty())
	{
		triggerRenderable->position(position);
		triggerRenderable->rotation(rotation);
		triggerRenderable->scale(scale);
	}
	else
	{
		XMMATRIX boneSpace = bonesTransformation.at(bone);
		XMMATRIX boneWorldSpace = XMMatrixMultiply(XMMatrixTranspose(boneSpace), world);
		XMVECTOR boneWorldScale, boneWorldRotationQ, boneWorldTranslation;
		XMMatrixDecompose(&boneWorldScale, &boneWorldRotationQ, &boneWorldTranslation, boneWorldSpace);

		XMVECTOR tPos = XMVector3Transform(XMLoadFloat3(&position), boneWorldSpace);
		XMVECTOR tScl = XMLoadFloat3(&scale);

		XMVECTOR tRotQ = XMQuaternionMultiply(boneWorldRotationQ, XMQuaternionRotationRollPitchYaw(
			XMConvertToRadians(rotation.x),
			XMConvertToRadians(rotation.y),
			XMConvertToRadians(rotation.z))
		);

		XMFLOAT3 fPos, fRot, fScl;
		fRot = Quaternion2Euler(tRotQ); //we convert to euler without much purpose, but anyway
		XMStoreFloat3(&fPos, tPos);
		XMStoreFloat3(&fScl, XMVectorMultiply(tScl, boneWorldScale));
		triggerRenderable->position(fPos);
		triggerRenderable->rotation(fRot);
		triggerRenderable->rotationQ(tRotQ);
		triggerRenderable->scale(fScl);
	}
}
