#pragma once
#include <string>
#include <functional>
#include <Application.h>
#include <string_view>
#include <imgui_internal.h>
#include <filesystem>
#include <UUID.h>
#include <NoStd.h>
#include <nlohmann/json.hpp>
#include <map>

#if defined(_EDITOR)

inline ImU32 rgba(auto r, auto g, auto b, auto a)
{
	return IM_COL32(
		static_cast<unsigned int>(r),
		static_cast<unsigned int>(g),
		static_cast<unsigned int>(b),
		static_cast<unsigned int>(a * 255)
	);
}

inline ImU32 rgba(nlohmann::json f4)
{
	return IM_COL32(
		static_cast<unsigned int>(f4.at(0) * 255),
		static_cast<unsigned int>(f4.at(1) * 255),
		static_cast<unsigned int>(f4.at(2) * 255),
		static_cast<unsigned int>(f4.at(3) * 255)
	);
}

namespace Editor
{
	extern bool NonGameMode;
	extern void OpenTemplateOnNextFrame(JUUID uuid);
	extern void OpenSceneObjectOnNextFrame(JUUID uuid);
};

namespace ImGui
{
	enum ItemLabelFlag
	{
		Left = 1u << 0u,
		Right = 1u << 1u,
		Default = Left,
	};

	template<typename T>
	bool DrawComboSelection(T& selected, std::map<std::string, T> selectables, std::function<void(std::string)> onSelect, std::string label = "##")
	{
		bool ret = false;
		std::string currentStr;
		for (auto it = selectables.begin(); it != selectables.end(); it++)
		{
			if (it->second == selected)
			{
				currentStr = it->first;
				break;
			}
		}
		if (ImGui::BeginCombo(label.c_str(), currentStr.c_str()))
		{
			Editor::NonGameMode = true;
			for (auto it = selectables.begin(); it != selectables.end(); it++)
			{
				if (ImGui::Selectable((it->first == "") ? "##" : it->first.c_str(), (currentStr == it->first) ? ImGuiSelectableFlags_Highlight : 0)) {
					ret = true;
					onSelect(it->first);
				}
				if (currentStr == it->first)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		return ret;
	}
	bool DrawComboSelection(JUUIDName selected, std::vector<JUUIDName> selectables, std::function<void(JUUIDName)> onSelect, std::string label = "##");
	bool DrawComboSelection(std::string selected, std::vector<std::string> selectables, std::function<void(std::string)> onSelect, std::string label = "##");
	bool DrawComboSelection(nlohmann::json& json, std::string attribute, std::vector<std::string> selectables, std::string label = "##");

	void ItemLabel(std::string_view title, ItemLabelFlag flags);

	void DrawItemWithEnabledState(std::function<void()> draw, bool enabled);

	void DrawTextureImage(ImTextureID textureId, unsigned int textureWidth, unsigned int textureHeight);

	inline bool DrawFromCombo(nlohmann::json& json, const std::string attribute, auto& listMap, std::string label = "")
	{
		std::string value = json.at(attribute);
		std::vector<std::string> selectables = nostd::GetKeysFromMap(listMap);
		bool ret = false;
		DrawComboSelection(value, selectables, [&json, &attribute, &ret](std::string newValue)
			{
				json[attribute] = newValue;
				ret = true;
			},
			label.c_str()
		);
		return ret;
	}

	bool DrawJsonCheckBox(nlohmann::json& json, const std::string attribute);

	bool DrawFromFloat(nlohmann::json& json, const std::string attribute, std::string label = "");

	bool DrawFromInt(nlohmann::json& json, const std::string attribute, std::string label = "");

	bool DrawFromUInt(nlohmann::json& json, const std::string attribute, std::string label = "");

	void DrawDynamicArray(
		std::string label,
		nlohmann::json& arr,
		std::function<void(nlohmann::json&, unsigned int)> insert,
		std::function<void(nlohmann::json&, unsigned int)> remove,
		std::function<void(nlohmann::json&, unsigned int)> draw,
		unsigned int maxItems,
		unsigned int minItems = 0U
	);

	bool DrawJsonInputText(nlohmann::json& json, std::string att);
	bool OpenFileDialog(std::wstring& path, std::wstring defaultDirectory, std::wstring defaultFileName, std::vector<std::pair<std::wstring, std::wstring>>& specs);
	void OpenFile(std::function<void(std::filesystem::path)> onFileSelected, std::string defaultDirectory, std::vector<std::string> filterName = { "JSON files. (*.json)" }, std::vector<std::string> filterPattern = { "*.json" }, bool detach = false);
	void OpenTemplate(const char* iconCode, JUUIDName uuidName);
	void OpenSceneObject(const char* iconCode, JUUIDName uuidName);

	void DrawAnimationController(
		std::function<bool()> animationsArePlaying,
		std::function<void(bool)> setPlayAnimation,
		std::function<void(float)> setAnimationTime,
		std::function<float()> getAnimationTimeFactor,
		std::function<void(float)> setAnimationTimeFactor,
		std::function<void()> gotoPrevAnimation,
		std::function<void()> gotoNextAnimation,
		std::function<bool()> animationsAreLooping,
		std::function<void(bool)> setAnimationLoop
	);
	void DrawAudioController(
		std::function<bool()> isPlaying,
		std::function<bool()> isPaused,
		std::function<void()> play,
		std::function<void()> stop,
		std::function<void()> pause,
		std::function<float()> getTime,
		std::function<float()> getDuration
	);
}
#endif
