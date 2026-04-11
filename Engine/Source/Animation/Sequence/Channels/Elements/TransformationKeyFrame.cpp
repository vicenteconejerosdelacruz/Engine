#include "pch.h"
#include "TransformationKeyFrame.h"

XMMATRIX TransformationKeyFrame::ToMatrix()
{
	float roll, pitch, yaw;
	pitch = rotation.x; yaw = rotation.y; roll = rotation.z;
	XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
	XMMATRIX rotationM = XMMatrixRotationQuaternion(rotQ);
	XMMATRIX scaleM = XMMatrixScalingFromVector({ scale.x, scale.y, scale.z });
	XMMATRIX positionM = XMMatrixTranslationFromVector({ position.x, position.y, position.z });
	return XMMatrixMultiply(XMMatrixMultiply(scaleM, rotationM), positionM);
}

bool TransformationKeyFrame::operator==(const TransformationKeyFrame& other) const
{
	return
		position.x == other.position.x
		&& position.y == other.position.y
		&& position.z == other.position.z
		&& rotation.x == other.rotation.x
		&& rotation.y == other.rotation.y
		&& rotation.z == other.rotation.z
		&& scale.x == other.scale.x
		&& scale.y == other.scale.y
		&& scale.z == other.scale.z
		&& easing == other.easing;
}