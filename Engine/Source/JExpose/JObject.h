#pragma once

#include <nlohmann/json.hpp>
#include <functional>
#include <any>
#include <map>
#include <UUID.h>
#include <v8-context.h>

struct JObject;

//typedef std::function<void(JUUID)> JObjectChangeCallback;
//typedef std::function<void(unsigned int, unsigned int)> JObjectChangePostCallback;

struct JObject : nlohmann::json
{
	virtual ~JObject() = default;

	std::map<std::string, std::tuple<size_t, bool>> UpdateFlagsMap;
	std::map<std::string, nlohmann::json> UpdatePrevValues;
	unsigned int updateFlag = 0U;

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
	virtual void WriteJson(nlohmann::json& j) {}

	bool dirty(size_t flag) const
	{
		return !!(updateFlag & (1 << flag));
	}

	void flag(size_t flag)
	{
		updateFlag |= (1 << flag);
	}

	void clean(size_t flag)
	{
		updateFlag &= ~(1 << flag);
	}

	void clear()
	{
		updateFlag = 0ULL;
	}

	virtual std::function<bool(JObject*)> GetAssetsConditioner() { return [](JObject*) { return true; }; }

	virtual void EditorPreview(size_t flags) {}
	virtual void DestroyEditorPreview() {}

	/*
	std::unordered_map<JUUID, std::tuple<JObjectChangeCallback, JObjectChangePostCallback>> bindedChangesCallbacks;
	*/
	/*
	void BindChangeCallback(JUUID objectUUID = "", JObjectChangeCallback cb = nullptr, JObjectChangePostCallback postCb = nullptr)
	{
		if (objectUUID == "" || (cb == nullptr && postCb == nullptr)) return;
		bindedChangesCallbacks.insert_or_assign(objectUUID, std::make_tuple(cb, postCb));
	}
	*/
	/*
	void UnbindChangeCallback(JUUID objectUUID)
	{
		if (objectUUID == "") return;
		bindedChangesCallbacks.erase(objectUUID);
	}
	*/
	/*
	static inline void RunChangesCallback(auto JObjectContainer, auto cbComplete)
	{
		unsigned int total = 0U;
		std::for_each(JObjectContainer.begin(), JObjectContainer.end(), [&total, cbComplete](auto j) mutable
			{
				for (auto& [_, lambdas] : j->bindedChangesCallbacks)
				{
					auto& lambda = std::get<0>(lambdas);
					if (lambda)
						lambda(j());
					total++;
				}
				cbComplete(j);
			}
		);

		unsigned int idx = 0;
		std::for_each(JObjectContainer.begin(), JObjectContainer.end(), [&idx, total](auto j)
			{
				for (auto& [_, lambdas] : j->bindedChangesCallbacks)
				{
					auto& lambda = std::get<1>(lambdas);
					if (lambda)
						lambda(idx, total);
					idx++;
				}
			}
		);
	}
	*/
};
