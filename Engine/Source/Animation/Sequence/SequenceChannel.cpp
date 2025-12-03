#include "pch.h"
#include "SequenceChannel.h"

nlohmann::json SequenceChannel::json()
{
	nlohmann::json j = nlohmann::json({});
	j["name"] = name;
	j["elements"] = nlohmann::json::array();
	for (auto& e : elements)
	{
		j["elements"].push_back(e.json());
	}
	return j;
}

bool SequenceChannel::operator==(const SequenceChannel& other) const {
	if (name != other.name) return false;
	return elements == other.elements;
}

SequenceChannel::SequenceChannel()
{
}

SequenceChannel::SequenceChannel(std::string name)
{
	this->name = name;
}

SequenceChannel::SequenceChannel(const nlohmann::json& j)
{
	name = j.at("name");
	nlohmann::json jelements = j.at("elements");
	for (size_t i = 0ULL; i < jelements.size(); i++)
	{
		nlohmann::json& element = jelements.at(i);
		elements.push_back(element);
	}
}

SequenceChannel::SequenceChannel(const SequenceChannel& seqChannel)
{
	name = seqChannel.name;
	elements = seqChannel.elements;
}

bool SequenceChannel::ChannelHasElementAtFrame(int frame)
{
	for (auto& element : elements)
	{
		bool left, right;
		if (element.ElementInFrame(frame, left, right))
			return true;
	}

	return false;
}

int SequenceChannel::GetAvailableFramesToLeft(int elementIndex)
{
	ChannelElement& element = elements.at(elementIndex);
	if (elementIndex == 0)
		return element.GetFrameStart();

	ChannelElement& prevElement = elements.at(elementIndex - 1);
	return element.GetFrameStart() - prevElement.GetFrameEnd() - 1;
}

int SequenceChannel::GetAvailableFramesToRight(int elementIndex, int totalFrames)
{
	ChannelElement& element = elements.at(elementIndex);
	if (elementIndex == (elements.size() - 1))
		return totalFrames - element.GetFrameEnd();

	ChannelElement& nextElement = elements.at(elementIndex + 1);
	return nextElement.GetFrameStart() - element.GetFrameEnd() - 1;
}

int SequenceChannel::GetFirstElementIndexBetweenFrames(int frameStart, int frameEnd)
{
	for (int i = 0; i < elements.size(); i++)
	{
		ChannelElement& element = elements.at(i);
		int elemStart = element.GetFrameStart();
		int elemEnd = element.GetFrameEnd();
		if (nostd::in_between(frameStart, elemStart, elemEnd) || nostd::in_between(frameEnd, elemStart, elemEnd))
			return i;
	}
	return -1;
}

int SequenceChannel::GetElementIndexBeforeFrame(int frame)
{
	int idx = -1;
	for (int i = 0; i < elements.size(); i++)
	{
		ChannelElement& element = elements.at(i);
		int elemEnd = element.GetFrameEnd();
		if (elemEnd < frame)
			break;
		idx++;
	}
	return idx;
}

void SequenceChannel::InsertChannelElement(ChannelElement element, int& totalFrames)
{
	int elemStart = element.GetFrameStart();
	int elemEnd = element.GetFrameEnd();
	int curIndex = GetFirstElementIndexBetweenFrames(elemStart, elemEnd);
	if (curIndex == -1)
	{
		int beforeIndex = GetElementIndexBeforeFrame(elemStart);
		if (beforeIndex == -1)
		{
			elements.push_back(element);
		}
		else
		{
			elements.insert(elements.begin() + beforeIndex, 1, element);
		}
		totalFrames = std::max(totalFrames, element.GetFrameEnd());
	}
	else
	{
		ChannelElement& curElem = elements.at(curIndex);
		int rightShift = elemEnd - curElem.GetFrameStart() + 1;

		int framesToRight = GetAvailableFramesToRight(static_cast<int>(elements.size()) - 1, totalFrames);
		if (rightShift > framesToRight)
		{
			totalFrames += (rightShift - framesToRight);
		}

		for (int i = curIndex; i < elements.size(); i++)
		{
			elements.at(i).Move(rightShift, totalFrames);
		}
		elements.insert(elements.begin() + curIndex, 1, element);
	}
}

void SequenceChannel::MoveElement(int elementIndex, int frames, int totalFrames)
{
	if (frames < 0)
	{
		frames = std::max(frames, -GetAvailableFramesToLeft(elementIndex));
		elements.at(elementIndex).Move(frames, totalFrames);
	}
	else if (frames > 0)
	{
		frames = std::min(frames, GetAvailableFramesToRight(elementIndex, totalFrames));
		elements.at(elementIndex).Move(frames, totalFrames);
	}
}

void SequenceChannel::DragElementLeftBoundary(int elementIndex, int frames, int totalFrames)
{
	ChannelElement& element = elements.at(elementIndex);
	if (frames < 0)
	{
		frames = -std::min(-frames, GetAvailableFramesToLeft(elementIndex));
		element.ExpandLeftBorder(frames);
	}
	else if (frames > 0)
	{
		int availableFrames = element.GetFrameEnd() - element.GetFrameStart();
		frames = std::min(frames, availableFrames);
		element.ExpandLeftBorder(frames);
	}
}

void SequenceChannel::DragElementRightBoundary(int elementIndex, int frames, int totalFrames)
{
	ChannelElement& element = elements.at(elementIndex);
	if (frames < 0)
	{
		int availableFrames = element.GetFrameEnd() - element.GetFrameStart();
		frames = std::max(frames, -availableFrames);
		element.ExpandRightBorder(frames);
	}
	else if (frames > 0)
	{
		frames = std::min(frames, GetAvailableFramesToRight(elementIndex, totalFrames));
		element.ExpandRightBorder(frames);
	}
}

void SequenceChannel::EraseElement(int elementIndex)
{
	nostd::vector_erase_index(elements, elementIndex);
}

void SequenceChannel::SplitElement(int elementIndex, int frame)
{
	ChannelElement& element = elements.at(elementIndex);

	auto [left, right] = element.Split(frame);

	elements.erase(elements.begin() + elementIndex);
	std::vector<ChannelElement> toInsert = { left,right };
	elements.insert(elements.begin() + elementIndex, toInsert.begin(), toInsert.end());
}

bool SequenceChannel::FrameHasElement(int frame, bool& leftBounded, bool& rightBounded)
{
	return std::any_of(elements.begin(), elements.end(), [&leftBounded, &rightBounded, frame](ChannelElement& elem)
		{
			return elem.ElementInFrame(frame, leftBounded, rightBounded);
		}
	);
}

bool SequenceChannel::FrameHasTransformationKeyframe(int frame)
{
	if (!ChannelHasElementAtFrame(frame)) return false;

	SequenceChannelElementTransformation* transformation = GetTransformationElementAtFrame(frame);
	if (!transformation) return false;

	return transformation->keyFrames.contains(frame);
}

void SequenceChannel::EraseElementInFrame(int frame)
{
	EraseElement(GetFirstElementIndexBetweenFrames(frame, frame));
}

void SequenceChannel::SplitElementInFrame(int frame)
{
	SplitElement(GetFirstElementIndexBetweenFrames(frame, frame), frame);
}

SequenceChannelElementAnimation* SequenceChannel::GetAnimationElementAtFrame(int frame)
{
	SequenceChannelElementAnimation* curr = nullptr;
	for (int i = 0; i < elements.size(); i++)
	{
		ChannelElement& element = elements.at(i);
		if (element.type != SCET_Animation)
			continue;
		if (frame < element.GetFrameStart())
			continue;
		if (frame >= element.GetFrameStart())
			curr = &element.animation;
	}
	return curr;
}

SequenceChannelElementTransformation* SequenceChannel::GetTransformationElementAtFrame(int frame)
{
	SequenceChannelElementTransformation* curr = nullptr;
	for (int i = 0; i < elements.size(); i++)
	{
		ChannelElement& element = elements.at(i);
		if (element.type != SCET_Transformation)
			continue;
		if (frame < element.GetFrameStart())
			continue;
		if (frame >= element.GetFrameStart())
			curr = &element.transformation;
	}
	return curr;
}

TransformationKeyFrame* SequenceChannel::GetTransformationKeyframe(int frame)
{
	auto* t = GetTransformationElementAtFrame(frame);
	return (t != nullptr && t->keyFrames.contains(frame)) ? &t->keyFrames.at(frame) : nullptr;
}

SequenceChannelElementSoundFX* SequenceChannel::GetSoundFXToCreateAtFrame(int frame)
{
	for (auto& element : elements)
	{
		if (element.type != SCET_SoundFX) continue;
		if (element.GetFrameStart() == frame) return &element.soundfx;
	}
	return nullptr;
}

SequenceChannelElementScript* SequenceChannel::GetScriptToRunAtFrame(int frame)
{
	for (auto& element : elements)
	{
		if (element.type == SCET_Script && element.GetFrameStart() == frame)
			return &element.script;
	}
	return nullptr;
}