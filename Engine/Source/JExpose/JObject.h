#pragma once

#include <nlohmann/json.hpp>
#include <functional>
#include <any>
#include <map>
#include <UUID.h>
#include <v8-context.h>
#include <ScriptBinding.h>

struct JObject : nlohmann::json
{
	virtual ~JObject() = default;

	std::map<std::string, std::tuple<size_t, bool>> UpdateFlagsMap;
	std::map<std::string, nlohmann::json> UpdatePrevValues;
	size_t updateFlag = 0U;

	JObject(nlohmann::json& json) :nlohmann::json(json) {}
	nlohmann::json json()
	{
		nlohmann::json j;
		j.merge_patch(*this);
		return j;
	}
	virtual void JUpdate(nlohmann::json p)
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
	virtual void JPatch(nlohmann::json p)
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

#if defined(_EDITOR)
	virtual void WriteJson(nlohmann::json& j) {}
#endif

	bool dirty(size_t flag) const
	{
		return !!(updateFlag & (1ULL << flag));
	}

	bool dirty(std::vector<size_t> flags)
	{
		return std::any_of(flags.begin(), flags.end(), [&](size_t flag) { return dirty(flag); });
	}

	void flag(size_t flag)
	{
		updateFlag |= (1ULL << flag);
	}

	void clean(size_t flag)
	{
		updateFlag &= ~(1ULL << flag);
	}

	void clean(std::vector<size_t> flags)
	{
		std::for_each(flags.begin(), flags.end(), [&](size_t flag) { clean(flag); });
	}

	void clear()
	{
		updateFlag = 0ULL;
	}

#if defined(_EDITOR)
	virtual std::function<bool(JObject*)> GetAssetsConditioner() { return [](JObject*) { return true; }; }
	virtual void EditorPreview(size_t flags) {}
	virtual void DestroyEditorPreview() {}
	virtual std::map<std::string, ScriptBinding> GetScriptBindingOptions() { return { {"",ScriptBinding()} }; }
#endif
};
