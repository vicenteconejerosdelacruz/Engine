#pragma once

#include <vector>
#include <set>
#include <string>
#include <functional>
#include <UUID.h>
#include <nlohmann/json.hpp>
#include <StepTimer.h>
#include <JTemplate.h>
#include <JTypes.h>

namespace Templates {

	std::set<JUUID>& GetTemplates(TemplateType type);
	std::unordered_map<JUUID, TemplateType>& GetTemplatesTypes();
	TemplateType GetTemplateType(JUUID uuid);
	bool TemplateExists(JUUID uuid);

	void CreateSystemTemplates();
	void CreateTemplates();

	/*
#if defined(_EDITOR)
	void SaveTemplates(const std::string folder, const std::string fileName, std::function<void(nlohmann::json&)> writer);
#endif
	*/
	void LoadTemplates(nlohmann::json templates, std::function<void(nlohmann::json&)> loader);
	void LoadTemplates(const std::string folder, const std::string fileName, std::function<void(nlohmann::json&)> loader);
	void DestroyTemplates();
	/*
#if defined(_EDITOR)
	void DestroyTemplatesReferences();
#endif
	void FreeGPUIntermediateResources();
	*/

	template<TemplateType T, typename J>
	inline void CreateJsonTemplate(nlohmann::json& json, auto getTypesTemplates)
	{
		JUUID uuid = json.at("uuid");
		JNAME name = json.at("name");

		auto& uuidSet = GetTemplates(T);
		auto& typesMap = GetTemplatesTypes();
		auto& templates = getTypesTemplates();

		if (templates.contains(uuid) || uuidSet.contains(uuid) || typesMap.contains(uuid))
		{
			assert(!!!"creation collision");
		}

		std::unique_ptr<J> jT = std::make_unique<J>(json);
		templates.insert_or_assign(uuid, std::make_tuple(name, std::move(jT)));
		uuidSet.insert(uuid);
		typesMap.insert_or_assign(uuid, T);
	}

	void WriteJsonTemplate(nlohmann::json& json, auto& Ts)
	{
		for (auto& t : Ts)
		{
			auto& tup = std::get<1>(t);
			auto& j = std::get<1>(tup);
			if (j->contains("systemCreated") && j->at("systemCreated") == true) continue;
			json.push_back(j->json());
		}
		std::sort(json.begin(), json.end(), [](auto j1, auto j2)
			{
				return j1.at("uuid") < j2.at("uuid");
			}
		);
	}

	void TemplatesStep(DX::StepTimer& timer);

	JTemplate* GetJTemplatePointer(JUUID uuid);

#if defined(_EDITOR)
	std::vector<JUUIDName> GetTemplatesTypesList();

	std::vector<std::pair<std::string, JsonToEditorValueType>> GetTemplateAttributes(TemplateType t);
	std::map<std::string, JEdvEditorDrawerFunction> GetTemplateDrawers(TemplateType t);
	std::map<std::string, JEdvEditorDrawerFunction> GetTemplatePreviewers(TemplateType t);
	/*
	std::vector<std::string> GetTemplateRequiredAttributes(TemplateType t);
	nlohmann::json GetTemplateJson(TemplateType t);
	nlohmann::json GetTemplateCreationModalProperties(TemplateType t);
	std::map<std::string, JEdvCreatorDrawerFunction> GetTemplateCreatorDrawers(TemplateType t);
	std::map<std::string, JEdvCreatorValidatorFunction> GetTemplateValidators(TemplateType t);

	TemplateType GetTemplateTypeFromFile(std::string file);
	std::string GetTemplateName(TemplateType t, std::string uuid);
	void CreateTemplate(TemplateType t, nlohmann::json json);
	std::string GetTemplateFile(TemplateType t);
	*/
	void DeleteTemplate(TemplateType t, std::string uuid);
	void DeleteTemplate(std::string uuid);
	/*
	void DeleteTemplateReferences(std::vector<nlohmann::json> references);
	void DeleteTemplateReferencesInLevels(std::vector<nlohmann::json> references);
	void DeleteTemplateReferencesInCurrentLevel(std::vector<nlohmann::json> references);

	void FindTemplatesReferencesInTemplates(std::string uuid, std::set<std::string> skipTemplateFile, std::function<void(nlohmann::json)> addReference);
	void FindTemplatesReferencesInLevels(std::string uuid, std::set<std::string> skipLevelFiles, std::function<void(nlohmann::json)> addReference);
	void FindTemplatesReferencesInCurrentLevel(std::string uuid, std::function<void(nlohmann::json)> addReference);
	void FindRecursiveJsonReference(nlohmann::json json, std::string uuid, std::string path, std::function<void(std::string path)> addReference);
	*/
#endif
}
