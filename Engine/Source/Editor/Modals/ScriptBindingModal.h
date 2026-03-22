#pragma once
#include <functional>
#include <ScriptBinding.h>
#include <SceneObject.h>
#include <AssetsTree.h>
#include "SimpleModal.h"

struct ScripBindingModal : SimpleModal
{
	void Init(
		ScriptBinding sb,
		JObject* object,
		std::function<std::vector<JUUIDName>()> getSceneObjects,
		std::function<SceneObject* (JUUID)> getSceneObjectPtr,
		std::function<void(nlohmann::json)> selector
	);
	void Draw();
	void DrawAssetTreeSelector(ImVec2 size);
	void DrawAssetResourceSelector(ImVec2 size);

	ScriptBinding binding;
	JObject* object;
	AssetsTree assetsTree;
	std::map<std::string, ScriptBinding> selectables = { {"",ScriptBinding()} };
	std::function<void(nlohmann::json)> selector;
};