#include "pch.h"
#include "JObject.h"
#include <NoV8.h>

JObject::JObject(nlohmann::json& json) :nlohmann::json(json)
{
}

nlohmann::json JObject::json()
{
	nlohmann::json j;
	j.merge_patch(*this);
	return j;
}


std::unordered_map<std::string, size_t> JObject::GetStringToFlags()
{
	return {};
}

void JObject::JUpdate(nlohmann::json p)
{
	UpdatePrevValues.clear();
	for (auto& [key, value] : p.items())
	{
		if (UpdateFlagsMap.contains(key))
		{
			UpdatePrevValues.insert_or_assign(key, at(key));
			bool update = std::get<1>(UpdateFlagsMap.at(key));
			if (!update) continue;
			size_t flag = std::get<0>(UpdateFlagsMap.at(key));
			updateFlag |= flag;
		}
	}
	merge_patch(p);
}

void JObject::JPatch(nlohmann::json p)
{
	UpdatePrevValues.clear();
	for (auto& [index, value] : p.items())
	{
		std::string path = value.at("path");
		std::vector<std::string> parts = nostd::split(path, "\\/");
		std::string key = parts[1];
		UpdatePrevValues.insert_or_assign(key, at(key));
		bool update = std::get<1>(UpdateFlagsMap.at(key));
		if (!update) continue;
		size_t flag = std::get<0>(UpdateFlagsMap.at(key));
		updateFlag |= flag;
	}
	patch_inplace(p);
}

bool JObject::dirty(size_t flag) const
{
	return !!(updateFlag & (1ULL << flag));
}

bool JObject::dirty(std::vector<size_t> flags)
{
	return std::any_of(flags.begin(), flags.end(), [&](size_t flag) { return dirty(flag); });
}

void JObject::flag(size_t flag)
{
	updateFlag |= (1ULL << flag);
}
void JObject::flag(std::vector<size_t> flags)
{
	std::for_each(flags.begin(), flags.end(), [&](size_t f) { flag(f); });
}
void JObject::flag(std::string key)
{
	flag(GetStringToFlags().at(key));
}

void JObject::clean(size_t flag)
{
	updateFlag &= ~(1ULL << flag);
}

void JObject::clean(std::vector<size_t> flags)
{
	std::for_each(flags.begin(), flags.end(), [&](size_t flag) { clean(flag); });
}

void JObject::clear()
{
	updateFlag = 0ULL;
}

void JObject::WriteJson(nlohmann::json& j)
{
}

std::function<bool(JObject*)> JObject::GetAssetsConditioner()
{
	return [](JObject*) { return true; };
}

void JObject::EditorPreview(size_t flags)
{
}

void JObject::DestroyEditorPreview()
{
}

std::map<std::string, ScriptBinding> JObject::GetScriptBindingOptions()
{
	return { {"",ScriptBinding()} };
}

const JPropertyMeta* JObject::GetMeta(const std::string& name) const
{
	auto it = propertyRegistry.find(name);
	return (it != propertyRegistry.end()) ? &it->second : nullptr;
}
