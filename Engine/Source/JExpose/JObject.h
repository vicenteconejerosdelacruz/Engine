#pragma once

#include <nlohmann/json.hpp>
#include <v8.h>
#include <ScriptBinding.h>
#include <NoV8.h>

#define DEF_STRING2FLAGS_FUNC(JClass,ParentJClass)\
std::unordered_map<std::string, size_t> GetStringToFlags() override {\
	std::unordered_map<std::string, size_t> joined = ParentJClass::GetStringToFlags();\
	joined.insert(StringToFlags.begin(), StringToFlags.end());\
	return joined;\
}
using namespace v8;
using namespace nov8;
struct JObject : nlohmann::json
{
	JObject(nlohmann::json& json);
	virtual ~JObject() = default;

	static std::string GetClassName() { return "JObject"; }
	virtual std::string GetJClassName() { return GetClassName(); }
	nlohmann::json json();

	virtual SceneUnitId Unit() { return 0ULL; }
	virtual std::unordered_map<std::string, size_t> GetStringToFlags();
	virtual void JUpdate(nlohmann::json p);
	virtual void JPatch(nlohmann::json p);

	bool dirty(size_t flag) const;
	bool dirty(std::vector<size_t> flags);
	void flag(size_t flag);
	void flag(std::vector<size_t> flags);
	void flag(std::string key);
	void clean(size_t flag);
	void clean(std::vector<size_t> flags);
	void clear();

#if defined(_EDITOR)
	virtual void WriteJson(nlohmann::json& j);
	virtual std::function<bool(JObject*)> GetAssetsConditioner();
	virtual void EditorPreview(size_t flags);
	virtual void DestroyEditorPreview();
	virtual std::map<std::string, ScriptBinding> GetScriptBindingOptions();
#endif

	//scripting
	static void RegisterScript(Isolate* isolate, Local<ObjectTemplate> proto, SceneUnitScripting* script) {}
	virtual void RegisterScriptInstance(Isolate* isolate, Local<ObjectTemplate> proto, SceneUnitScripting* script) { RegisterScript(isolate, proto, script); }
	const JPropertyMeta* GetMeta(const std::string& name) const;

	std::map<std::string, std::tuple<size_t, bool>> UpdateFlagsMap;
	std::map<std::string, nlohmann::json> UpdatePrevValues;
	size_t updateFlag = 0U;
	std::unordered_map<std::string, JPropertyMeta> propertyRegistry = {};
};
