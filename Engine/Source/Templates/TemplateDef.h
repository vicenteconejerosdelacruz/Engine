#define TEMPDEF_TUPLE(TemplateName) TemplateName##Templates TemplateName##templates

#define TEMPDEF_GETTEMPLATES(TemplateName) TemplateName##Templates& Get##TemplateName##Templates()\
{\
	return TemplateName##templates;\
}

#define TEMPDEF_CREATE(TemplateName) void Create##TemplateName(nlohmann::json& json)\
{\
	CreateJsonTemplate<TemplateName::templateType, TemplateName##Json>(json, Get##TemplateName##Templates);\
}

#define TEMPDEF_GET(TemplateName) std::unique_ptr<TemplateName##Json>& Get##TemplateName##Template(JUUID uuid)\
{\
	auto& tuple = TemplateName##templates.at(uuid);\
	auto& ptr = std::get<1>(tuple);\
	return ptr;\
}

#define TEMPDEF_GETUUIDNAMES(TemplateName) std::vector<JUUIDName> Get##TemplateName##sUUIDsNames()\
{\
	return GetUUIDsNames(TemplateName##templates);\
}

#define TEMPDEF_GETNAMES(TemplateName) std::vector<JNAME> Get##TemplateName##sNames()\
{\
	return GetNames(TemplateName##templates);\
}

#define TEMPDEF_GETNAME(TemplateName) JNAME Get##TemplateName##Name(JUUID uuid)\
{\
	return GetName(uuid, Get##TemplateName##Templates);\
}

#define TEMPDEF_GETUUIDBYNAME(TemplateName) JUUID Get##TemplateName##UUIDByName(JNAME name)\
{\
	return GetUUIDByName(name, Get##TemplateName##Templates);\
}

#define TEMPDEF_WRITEJSON(TemplateName) void Write##TemplateName##sJson(nlohmann::json& json)\
{\
	WriteJsonTemplate(json, TemplateName##templates);\
}

#define TEMPDEF_RELEASE(TemplateName) void Release##TemplateName##Templates()\
{\
	TemplateName##templates.clear();\
}

#define TEMPDEF_EXIST(TemplateName) bool TemplateName##TemplateExist(JUUID uuid)\
{\
	return TemplateName##templates.contains(uuid);\
}

#define TEMPDEF_RENAME(TemplateName) void Rename##TemplateName##Template(JUUID uuid,std::string newName)\
{\
	auto& tup = TemplateName##templates.at(uuid);\
	auto& refName = std::get<0>(tup);\
	auto& ptr = std::get<1>(tup);\
	refName = newName;\
	ptr->name(newName);\
}

#define TEMPDEF_DELETE(TemplateName) void Delete##TemplateName##Template(JUUID uuid)\
{\
	TemplateName##templates.erase(uuid);\
}\

#define TEMPDEF_FULL(TemplateName) \
	TEMPDEF_TUPLE(TemplateName);\
	TEMPDEF_GETTEMPLATES(TemplateName);\
	TEMPDEF_CREATE(TemplateName);\
	TEMPDEF_GET(TemplateName);\
	TEMPDEF_GETUUIDNAMES(TemplateName);\
	TEMPDEF_GETNAMES(TemplateName);\
	TEMPDEF_GETNAME(TemplateName);\
	TEMPDEF_GETUUIDBYNAME(TemplateName);\
	TEMPDEF_WRITEJSON(TemplateName);\
	TEMPDEF_RELEASE(TemplateName);\
	TEMPDEF_EXIST(TemplateName);\
	TEMPDEF_RENAME(TemplateName);\
	TEMPDEF_DELETE(TemplateName)

#define TEMPDEF_REFTRACKER(TemplateName) static RefTracker<JUUID, std::unique_ptr<TemplateName##Instance>> refTracker;\
static std::mutex refTrackMutex;\
std::unique_ptr<TemplateName##Instance>& Create##TemplateName##Instance(JUUID templateUUID, std::function<std::unique_ptr<TemplateName##Instance>()> newRefCallback)\
{\
	std::lock_guard<std::mutex> lock(refTrackMutex);\
	if (refTracker.Has(templateUUID))\
	{\
		std::unique_ptr<TemplateName##Instance>& instance = refTracker.FindValue(templateUUID);\
		refTracker.IncrementRefCount(templateUUID, 1U);\
		return instance;\
	}\
	else\
	{\
		return refTracker.AddRef(templateUUID, newRefCallback);\
	}\
}\
std::unique_ptr<TemplateName##Instance>& Create##TemplateName##Instance(JUUID templateUUID, JUUID instanceKey, std::function<std::unique_ptr<TemplateName##Instance>()> newRefCallback)\
{\
	std::lock_guard<std::mutex> lock(refTrackMutex);\
	if (refTracker.Has(instanceKey))\
	{\
		std::unique_ptr<TemplateName##Instance>& instance = refTracker.FindValue(instanceKey);\
		refTracker.IncrementRefCount(instanceKey, 1U);\
		return instance;\
	}\
	else\
	{\
		return refTracker.AddRef(instanceKey, newRefCallback);\
	}\
}\
std::unique_ptr<TemplateName##Instance>& Create##TemplateName##Instance(JUUID templateUUID)\
{\
	std::lock_guard<std::mutex> lock(refTrackMutex);\
	return Create##TemplateName##Instance(templateUUID, [templateUUID]\
		{\
			return std::make_unique<TemplateName##Instance>(templateUUID);\
		}\
	);\
}\
bool Delete##TemplateName##Instance(JUUID instanceKey)\
{\
	std::lock_guard<std::mutex> lock(refTrackMutex);\
	if (refTracker.Has(instanceKey))\
	{\
		refTracker.RemoveRef(instanceKey);\
		return true;\
	}\
	return false;\
}\
std::unique_ptr<TemplateName##Instance>& Get##TemplateName##Instance(JUUID instanceKey)\
{\
	std::lock_guard<std::mutex> lock(refTrackMutex);\
	return refTracker.FindValue(instanceKey);\
}\
void Clear##TemplateName##Instances()\
{\
	std::lock_guard<std::mutex> lock(refTrackMutex);\
	refTracker.Clear();\
}
