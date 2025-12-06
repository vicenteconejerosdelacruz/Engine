#include "pch.h"
#include "SequenceChannelElementTransformation.h"
#include <DirectXMath.h>
#include <NoStd.h>

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

SequenceChannelElementTransformation::SequenceChannelElementTransformation(const nlohmann::json& j) :SequenceChannelElement(j)
{
	nlohmann::json keyframes = j.at("keyframes");
	for (int i = 0; i < keyframes.size(); i++)
	{
		nlohmann::json keyframe = keyframes.at(i);
		int frame = keyframe.at("frame");
		TransformationKeyFrame tkey;
		tkey.position = ToXMFLOAT3(keyframe.at("position"));
		tkey.rotation = ToXMFLOAT3(keyframe.at("rotation"));
		tkey.scale = ToXMFLOAT3(keyframe.at("scale"));
		tkey.easing = StringToEasing.at(keyframe.at("easing"));
		keyFrames.insert_or_assign(frame, tkey);
	}
}

bool SequenceChannelElementTransformation::operator==(const SequenceChannelElementTransformation& other) const {
	return keyFrames == other.keyFrames;
}

nlohmann::json SequenceChannelElementTransformation::json()
{
	auto toJsonKeyFrames = [](std::unordered_map<int, TransformationKeyFrame> t)
		{
			nlohmann::json arr = nlohmann::json::array();
			for (auto& [frame, keyframe] : t)
			{
				nlohmann::json jk = {
					{ "frame", frame },
					{ "position", FromXMFLOAT3(keyframe.position) },
					{ "rotation", FromXMFLOAT3(keyframe.rotation) },
					{ "scale", FromXMFLOAT3(keyframe.scale) },
					{ "easing", EasingToString.at(keyframe.easing) },
				};
				arr.push_back(jk);
			}
			std::sort(arr.begin(), arr.end(), [](auto& a, auto& b)
				{
					return a.at("frame") < b.at("frame");
				}
			);
			return arr;
		};

	nlohmann::json j =
	{
		{ "frameStart", frameStart},
		{ "frameEnd" , frameEnd},
		{ "keyframes", toJsonKeyFrames(keyFrames) }
	};

	return j;
}

TransformationKeyFrame SequenceChannelElementTransformation::GetKeyFrameBeforeFrame(int frame)
{
	for (int i = frame - 1; i >= 0; i--)
	{
		if (keyFrames.contains(i))
			return keyFrames.at(i);
	}
	return TransformationKeyFrame();
}

XMMATRIX SequenceChannelElementTransformation::GetTransformationInFrame(int frame)
{
	if (keyFrames.contains(frame)) return keyFrames.at(frame).ToMatrix();
	if (!HasKeyframeBeforeFrame(frame)) return XMMatrixIdentity();
	if (!HasKeyframeAfterFrame(frame)) return GetTransformationBeforeFrame(frame);
	auto [frameA, keyframeA, frameB, keyframeB] = GetKeyframesBetweenFrame(frame);
	return InterpolateKeyframes(keyframeA, keyframeB, frame - frameA, frameB - frameA);
}

XMMATRIX SequenceChannelElementTransformation::GetTransformationBeforeFrame(int frame)
{
	return GetKeyFrameBeforeFrame(frame).ToMatrix();
}

bool SequenceChannelElementTransformation::HasKeyframeBeforeFrame(int frame)
{
	for (auto& [k, _] : keyFrames)
	{
		if (k < frame) return true;
	}
	return false;
}

bool SequenceChannelElementTransformation::HasKeyframeAfterFrame(int frame)
{
	for (auto& [k, _] : keyFrames)
	{
		if (k > frame) return true;
	}
	return false;
}

std::tuple<int, TransformationKeyFrame, int, TransformationKeyFrame> SequenceChannelElementTransformation::GetKeyframesBetweenFrame(int frame)
{
	int leftMinDiff = -1;
	int rightMinDiff = -1;
	for (auto& [k, v] : keyFrames)
	{
		if (k < frame)
			leftMinDiff = (leftMinDiff == -1) ? (frame - k) : std::min(leftMinDiff, frame - k);
		if (k > frame)
			rightMinDiff = (rightMinDiff == -1) ? (k - frame) : std::min(rightMinDiff, k - frame);
	}
	int leftFrame = frame - leftMinDiff;
	int rightFrame = frame + rightMinDiff;
	return std::make_tuple(leftFrame, keyFrames.at(leftFrame), rightFrame, keyFrames.at(rightFrame));
}

XMMATRIX SequenceChannelElementTransformation::InterpolateKeyframes(TransformationKeyFrame keyA, TransformationKeyFrame keyB, int frameAfterA, int framesBetweenKeyframes)
{
	std::unordered_map<Easing, std::function<float(float)>> ease =
	{
		{ Easing_Linear, [](float t) { return t; } },
		{ Easing_Sine_Ease_In, [](float t) { return static_cast<float>(-cos(t * M_PI_2) + 1.0f); } },
		{ Easing_Sine_Ease_Out,[](float t) { return static_cast<float>(sin(t * M_PI_2)); } },
		{ Easing_Sine_Ease_In_Out,[](float t) { return static_cast<float>(-0.5f * (cos(M_PI * t) - 1.0f)); } }
	};

	float t = ease.at(keyA.easing)(static_cast<float>(frameAfterA) / static_cast<float>(framesBetweenKeyframes));

	XMVECTOR pA({ keyA.position.x,keyA.position.y,keyA.position.z });
	XMVECTOR pB({ keyB.position.x,keyB.position.y,keyB.position.z });
	XMVECTOR pt = XMVectorLerp(pA, pB, t);

	float rollA, pitchA, yawA;
	float rollB, pitchB, yawB;
	pitchA = keyA.rotation.x; yawA = keyA.rotation.y; rollA = keyA.rotation.z;
	pitchB = keyB.rotation.x; yawB = keyB.rotation.y; rollB = keyB.rotation.z;
	XMVECTOR rA = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitchA), XMConvertToRadians(yawA), XMConvertToRadians(rollA));
	XMVECTOR rB = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitchB), XMConvertToRadians(yawB), XMConvertToRadians(rollB));
	XMVECTOR rt = XMQuaternionSlerp(rA, rB, t);

	XMVECTOR sA({ keyA.scale.x,keyA.scale.y,keyA.scale.z });
	XMVECTOR sB({ keyB.scale.x,keyB.scale.y,keyB.scale.z });
	XMVECTOR st = XMVectorLerp(sA, sB, t);

	XMMATRIX rotationM = XMMatrixRotationQuaternion(rt);
	XMMATRIX scaleM = XMMatrixScalingFromVector(st);
	XMMATRIX positionM = XMMatrixTranslationFromVector(pt);
	return XMMatrixMultiply(XMMatrixMultiply(scaleM, rotationM), positionM);
}

void SequenceChannelElementTransformation::CreateInterpolatedKeyFrame(int frame)
{
	std::vector<int> frames = nostd::GetKeysFromMap(keyFrames);
	std::sort(frames.begin(), frames.end());
	auto it_lower = std::lower_bound(frames.begin(), frames.end(), frame) - 1;
	auto it_upper = std::upper_bound(frames.begin(), frames.end(), frame);

	TransformationKeyFrame k0 = keyFrames.at(*it_lower);
	TransformationKeyFrame k1 = keyFrames.at(*it_upper);

	XMMATRIX M = InterpolateKeyframes(k0, k1, frame - *it_lower, *it_upper - *it_lower);
	XMVECTOR P, R, S;
	XMMatrixDecompose(&S, &R, &P, M);

	TransformationKeyFrame knew;
	knew.easing = k0.easing;
	XMStoreFloat3(&knew.position, P);
	XMStoreFloat3(&knew.scale, S);
	XMFLOAT3 rot = Quaternion2Euler(R);
	knew.rotation = rot;
	keyFrames.insert_or_assign(frame, knew);
}

void SequenceChannelElementTransformation::FlipKeyFrames()
{
	std::unordered_map<int, TransformationKeyFrame> newKeyFrames;

	for (auto& [frame, keyframe] : keyFrames)
	{
		int newFrame = frameEnd - frame - frameStart;
		newKeyFrames.insert_or_assign(newFrame, keyframe);
	}

	keyFrames = newKeyFrames;
}
