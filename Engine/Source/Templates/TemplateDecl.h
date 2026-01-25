#define TEMPDECL_TUPLE(TemplateName) typedef std::tuple<JNAME, std::unique_ptr<TemplateName##Json>> TemplateName##JsonTuple;\
typedef TemplatesContainer<TemplateName##JsonTuple> TemplateName##Templates
#define TEMPDECL_GETTEMPLATES(TemplateName) TemplateName##Templates& Get##TemplateName##Templates()
#define TEMPDECL_CREATE(TemplateName) void Create##TemplateName(nlohmann::json& json)
#define TEMPDECL_GET(TemplateName) std::unique_ptr<TemplateName##Json>& Get##TemplateName##Template(JUUID uuid)
#define TEMPDECL_GETUUIDNAMES(TemplateName) std::vector<JUUIDName> Get##TemplateName##sUUIDsNames()
#define TEMPDECL_GETNAMES(TemplateName) std::vector<JNAME> Get##TemplateName##sNames()
#define TEMPDECL_GETNAME(TemplateName) JNAME Get##TemplateName##Name(JUUID uuid)
#define TEMPDECL_GETUUIDBYNAME(TemplateName) JUUID Get##TemplateName##UUIDByName(JNAME name)
#define TEMPDECL_WRITEJSON(TemplateName) void Write##TemplateName##sJson(nlohmann::json& json)
#define TEMPDECL_RELEASE(TemplateName) void Release##TemplateName##Templates()
#define TEMPDECL_EXIST(TemplateName) bool TemplateName##TemplateExist(JUUID uuid)
#define TEMPDECL_RENAME(TemplateName) void Rename##TemplateName##Template(JUUID uuid, std::string newName)
#define TEMPDECL_DELETE(TemplateName) void Delete##TemplateName##Template(JUUID uuid)

#define TEMPDECL_FULL(TemplateName) \
	TEMPDECL_TUPLE(TemplateName);\
	TEMPDECL_GETTEMPLATES(TemplateName);\
	TEMPDECL_CREATE(TemplateName);\
	TEMPDECL_GET(TemplateName);\
	TEMPDECL_GETUUIDNAMES(TemplateName);\
	TEMPDECL_GETNAMES(TemplateName);\
	TEMPDECL_GETNAME(TemplateName);\
	TEMPDECL_GETUUIDBYNAME(TemplateName);\
	TEMPDECL_WRITEJSON(TemplateName);\
	TEMPDECL_RELEASE(TemplateName);\
	TEMPDECL_EXIST(TemplateName);\
	TEMPDECL_RENAME(TemplateName);\
	TEMPDECL_DELETE(TemplateName)

#define TEMPDECL_REFTRACKER(TemplateName)\
	std::unique_ptr<TemplateName##Instance>& Create##TemplateName##Instance(JUUID templateUUID, std::function<std::unique_ptr<TemplateName##Instance>()> newRefCallback);\
	std::unique_ptr<TemplateName##Instance>& Create##TemplateName##Instance(JUUID templateUUID, JUUID instanceKey, std::function<std::unique_ptr<TemplateName##Instance>()> newRefCallback);\
	std::unique_ptr<TemplateName##Instance>& Create##TemplateName##Instance(JUUID templateUUID);\
	bool Delete##TemplateName##Instance(JUUID instanceKey);\
	std::unique_ptr<TemplateName##Instance>& Get##TemplateName##Instance(JUUID instanceKey);\
	void Clear##TemplateName##Instances();

#define TEMPLATE_DECL(TemplateName)\
	TemplateName##Json(nlohmann::json& json);
