#pragma once

#include <queue>
#include <map>
#include <SimpleMath.h>
#include <assimp/scene.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <memory>

namespace Scene
{
	DEF_SCENEOBJECT_ID_DEP(Renderable);
};
struct aiScene;

using namespace DirectX;
using namespace Scene;

namespace Animation
{
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

	typedef std::map<std::string, BoneKeys> BonesKeysMap;
	typedef std::map<std::string, BonesKeysMap> AnimationBonesKeys;
	typedef std::map<std::string, std::string> ParentBones;
	typedef std::map<std::string, HierarchyNode*> BoneNodePointer;
	typedef std::map<std::string, XMMATRIX> NodeTransformsMap;

	typedef std::pair<HierarchyNode*, bool> MultiplyCmd;
	typedef std::queue<MultiplyCmd> MultiplyCmdQueue;

	struct Animated
	{
		AnimationLengthMap animationsLength;
		BonesTransformations bonesOffsets;
		XMMATRIX rootNodeInverseTransform;
		NodeTransformsMap globalNodeTransforms;

		AnimationBonesKeys animationsBonesKeys;
		HierarchyNode rootHierarchy;
		MultiplyCmdQueue multiplyNavigator;

		BoneNodePointer boneNodePointers;
		ParentBones bonesParents;
	};

	void BuildBonesOffsets(const aiScene* aiModel, BonesTransformations& bonesOffsets);
	void BuildAnimationBonesKeys(const aiScene* model, AnimationBonesKeys& animationBonesKeys);
	void BuildNodesHierarchy(aiNode* node, HierarchyNode* nodeInHierarchy, MultiplyCmdQueue& multiplyNavigator, BoneNodePointer& boneNodePointers, ParentBones& bonesParents, std::string parentName = "");
	void DestroyNodesHierarchy(HierarchyNode* node);
	std::unique_ptr<Animated> CreateAnimatedFromAssimp(const aiScene* aiModel);

	void DestroyAnimated();
	void DestroyAnimated(SceneUnitId id);

	void AttachAnimation(RenderableID renderable, std::unique_ptr<Animated>& animated);
	ConstantsBufferID GetAnimatedConstantsBuffer(RenderableID renderable);
	void WriteBoneTransformationsToConstantsBuffer(RenderableID renderable, BonesTransformations& bonesTransformation, unsigned int backbufferIndex);

	void TraverseMultiplycationQueue(float time, std::string currentAnimation, std::unique_ptr<Animated>& animations, BonesTransformations& bonesTransformation, BonesTransformations& sequenceBoneTransformations);
	void TraverseMultiplycationQueue(float time, MultiplyCmdQueue& cmds, BonesKeysMap& boneKeys, BonesTransformations& bonesTransformation, BonesTransformations& bonesOffsets, BonesTransformations& sequenceBoneTransformations, XMMATRIX& rootNodeInverseTransform, NodeTransformsMap& globalNodeTransforms, XMMATRIX parentTransformation);
	std::tuple<XMMATRIX, XMFLOAT3, XMFLOAT3, XMVECTOR, XMFLOAT3> GetBoneTransformation(XMMATRIX world, Animation::NodeTransformsMap& nodesTransformation, std::string bone);
}
