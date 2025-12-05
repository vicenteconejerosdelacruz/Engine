#pragma once

#include <queue>
#include <map>
#include <DirectXMath.h>
#include <assimp/scene.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <memory>

namespace Scene { struct Renderable; };
struct aiScene;

using namespace DirectX;

namespace Animation
{
	static const std::string AnimationConstantBufferName = "animation";
	static const unsigned int MAX_BONES = 1024U;
	typedef XMMATRIX BonesMatrices[MAX_BONES];

	using namespace DirectX;
	using namespace DeviceUtils;
	using namespace Scene;

	//keep as std::map as the order is very important for the hierarchy structure
	typedef std::map<std::string, float> AnimationLengthMap;
	typedef std::map<std::string, XMMATRIX> BonesTransformations;

	struct HierarchyNode
	{
		std::string name;
		XMMATRIX transformation;
		unsigned int numChildren = 0;
		HierarchyNode* children = nullptr;
	};

	struct KeyFrame
	{
		float time;
		XMVECTOR key;
	};

	struct BoneKeys
	{
		std::vector<KeyFrame> positions;
		std::vector<KeyFrame> scaling;
		std::vector<KeyFrame> rotation;
	};

	typedef std::unordered_map<std::string, BoneKeys> BonesKeysMap;
	typedef std::unordered_map<std::string, BonesKeysMap> AnimationBonesKeys;

	typedef std::pair<HierarchyNode*, bool> MultiplyCmd;
	typedef std::queue<MultiplyCmd> MultiplyCmdQueue;

	struct Animated
	{
		AnimationLengthMap animationsLength;
		BonesTransformations bonesOffsets;
		XMMATRIX rootNodeInverseTransform;

		AnimationBonesKeys animationsBonesKeys;
		HierarchyNode rootHierarchy;
		MultiplyCmdQueue multiplyNavigator;
	};

	void BuildBonesOffsets(const aiScene* aiModel, BonesTransformations& bonesOffsets);
	void BuildAnimationBonesKeys(const aiScene* model, AnimationBonesKeys& animationBonesKeys);
	void BuildNodesHierarchy(aiNode* node, HierarchyNode* nodeInHierarchy, MultiplyCmdQueue& multiplyNavigator);
	void DestroyNodesHierarchy(HierarchyNode* node);
	std::unique_ptr<Animated> CreateAnimatedFromAssimp(const aiScene* aiModel);

	void DestroyAnimated();

	void AttachAnimation(JUUID renderableUUID, std::unique_ptr<Animated>& animated);
	ConstantsBufferUUID GetAnimatedConstantsBuffer(JUUID renderableUUID);
	void WriteBoneTransformationsToConstantsBuffer(JUUID renderableUUID, BonesTransformations& bonesTransformation, unsigned int backbufferIndex);

	void TraverseMultiplycationQueue(float time, std::string currentAnimation, std::unique_ptr<Animated>& animations, BonesTransformations& bonesTransformation);
	void TraverseMultiplycationQueue(float time, MultiplyCmdQueue& cmds, BonesKeysMap& boneKeys, BonesTransformations& bonesTransformation, BonesTransformations& bonesOffsets, XMMATRIX& rootNodeInverseTransform, XMMATRIX parentTransformation);
}

