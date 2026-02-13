#if defined(JEXPOSE_ATT_DECL)

#define JCLASS(CLASS,GETJOBJECTS)

#define JTYPE(TYPE,VALUE) virtual TYPE JType() { return VALUE; }

#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) \
	void create_##ATT(TYPE t)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = t;\
		}\
	}\
	TYPE ATT()\
	{\
		return at(#ATT);\
	}\
	void ATT(TYPE v)\
	{\
		at(#ATT)=v;\
	}\

#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	void create_##ATT(TYPE t)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = FROMTYPE(t);\
		}\
	}\
	TYPE ATT()\
	{\
		if(contains(#ATT))\
		{\
			return TOTYPE(at(#ATT)); \
		}\
		else\
		{\
			return TYPE();\
		}\
	}\
	void ATT(TYPE v)\
	{\
		(*this)[#ATT] = FROMTYPE(v);\
	}\

#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) \
	void create_##ATT(TYPE v)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = TYPE##ToString.at(v);\
		}\
	}\
	TYPE ATT()\
	{\
		return StringTo##TYPE.at(at(#ATT));\
	}\
	void ATT(TYPE v)\
	{\
		at(#ATT)=TYPE##ToString.at(v);\
	}\

#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	void create_##ATT(std::vector<TYPE> vec)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = nlohmann::json::array();\
			for (auto& v : vec)\
			{\
				at(#ATT).push_back(v);\
			}\
		}\
	}\
	std::vector<TYPE> ATT()\
	{\
		std::vector<TYPE> vec;\
		if(contains(#ATT))\
		{\
			for (nlohmann::json::iterator it = at(#ATT).begin(); it != at(#ATT).end(); it++)\
			{\
				vec.push_back(*it);\
			}\
		}\
		return vec;\
	}\
	void ATT(std::vector<TYPE> vec)\
	{\
		at(#ATT) = nlohmann::json::array();\
		for (auto& v : vec)\
		{\
			at(#ATT).push_back(v);\
		}\
	}\
	void ATT##_push_back(TYPE v)\
	{\
		at(#ATT).push_back(v);\
	}

#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	void create_##ATT(std::vector<TYPE> vec)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = nlohmann::json::array();\
			for (auto& v : vec)\
			{\
				at(#ATT).push_back(FROMTYPE(v));\
			}\
		}\
	}\
	std::vector<TYPE> ATT()\
	{\
		std::vector<TYPE> vec;\
		if(contains(#ATT))\
		{\
			for (nlohmann::json::iterator it = at(#ATT).begin(); it != at(#ATT).end(); it++)\
			{\
				vec.push_back(TOTYPE(*it));\
			}\
		}\
		return vec;\
	}\
	void ATT(std::vector<TYPE> vec)\
	{\
		at(#ATT) = nlohmann::json::array();\
		for (auto& v : vec)\
		{\
			at(#ATT).push_back(FROMTYPE(v));\
		}\
	}\
	void ATT##_push_back(TYPE v)\
	{\
		at(#ATT).push_back(FROMTYPE(v));\
	}

#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	void create_##ATT(std::set<TYPE> attset)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = nlohmann::json::array();\
			for (auto& v : attset)\
			{\
				at(#ATT).push_back(v);\
			}\
		}\
	}\
	std::set<TYPE> ATT()\
	{\
		std::set<TYPE> s;\
		if(contains(#ATT))\
		{\
			for (auto& v : at(#ATT))\
			{\
				s.insert(static_cast<TYPE>(v)); \
			}\
		}\
		return s;\
	}\
	bool ATT##_contains(TYPE value)\
	{\
		for (auto& v : at(#ATT)) {\
			if (v == value) return true;\
		}\
		return false;\
	}\
	void ATT##_insert(TYPE value)\
	{\
		for (auto& v : at(#ATT)) {\
			if (v == value) return;\
		}\
		at(#ATT).push_back(value);\
	}\
	void ATT##_erase(TYPE value)\
	{\
		unsigned int idx = -1;\
		for(unsigned int i = 0 ; i < at(#ATT).size(); i++)\
		{\
			if(at(#ATT).at(i)==value)\
			{\
				idx = i;\
				break;\
			}\
		}\
		if (idx != -1) at(#ATT).erase(idx);\
	}\
	size_t ATT##_size()\
	{\
		return at(#ATT).size();\
	}\
	void ATT##_clear()\
	{\
		at(#ATT).clear();\
	}

#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	void create_##ATT(std::map<KEYTYPE,VALUETYPE> map)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = nlohmann::json::object({}); \
			for (auto& pair : map)\
			{\
				at(#ATT).merge_patch(FROMTYPE(pair)); \
			}\
		}\
	}\
	std::map<KEYTYPE, VALUETYPE> ATT()\
	{\
		std::map<KEYTYPE, VALUETYPE> map;\
		if(contains(#ATT))\
		{\
			for (nlohmann::json::iterator it = at(#ATT).begin(); it != at(#ATT).end(); it++)\
			{\
				map.insert(TOTYPE(it)); \
			}\
		}\
		return map;\
	}\
	void ATT(std::map<KEYTYPE,VALUETYPE> map)\
	{\
		(*this)[#ATT] = nlohmann::json::object({}); \
		for (auto& pair : map)\
		{\
			at(#ATT).merge_patch(FROMTYPE(pair)); \
		}\
	}\
	bool ATT##_contains(KEYTYPE k)\
	{\
		if(!contains(#ATT)) return false;\
		std::pair<KEYTYPE, VALUETYPE> pair(k,""); \
		nlohmann::json o = FROMTYPE(pair);\
		nlohmann::json::iterator it = o.begin();\
		return at(#ATT).contains(it.key());\
	}\
	void ATT##_insert(KEYTYPE k,VALUETYPE v)\
	{\
		std::pair<KEYTYPE, VALUETYPE> pair(k,v); \
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = nlohmann::json::object({}); \
		}\
		at(#ATT).merge_patch(FROMTYPE(pair)); \
	}\
	void ATT##_erase(KEYTYPE k,VALUETYPE v)\
	{\
		std::pair<KEYTYPE, VALUETYPE> pair(k,nullptr); \
		at(#ATT).merge_patch(FROMTYPE(pair)); \
	}

#define JEXPOSE_MAP_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	void create_##ATT(nlohmann::json m)\
	{\
		if (!contains(#ATT))\
		{\
			(*this)[#ATT]=m;\
		}\
		nlohmann::json& j = (*this)[#ATT];\
		std::map<std::string, std::string> objectMap;\
		for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)\
		{\
			objectMap.insert_or_assign(it.key(), Create##TYPE(it.key(), SUuuid(), it.value())); \
		}\
		for(auto &[key,uuid]:objectMap)\
		{\
			(*this)[#ATT][key] = uuid;\
		}\
	}\
	std::map<std::string, std::reference_wrapper<std::unique_ptr<TYPE>>> ATT()\
	{\
		std::map<std::string, std::reference_wrapper<std::unique_ptr<TYPE>>> m;\
		nlohmann::json& j = (*this)[#ATT];\
		for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)\
		{\
			m.insert_or_assign(it.key(), Get##TYPE(it.value())); \
		}\
		return m;\
	}\
	bool ATT##_contains(std::string key)\
	{\
		if(!contains(#ATT)) return false;\
		nlohmann::json& j = (*this)[#ATT];\
		return j.contains(key);\
	}\
	void ATT##_insert(std::string key, JUUID uuid)\
	{\
		(*this)[#ATT][key]=uuid;\
	}\
	void ATT##_erase(std::string key)\
	{\
		(*this)[#ATT].erase(key);\
	}

#define JEXPOSE_VECTOR_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	void create_##ATT(nlohmann::json m)\
	{\
		if (!contains(#ATT))\
		{\
			(*this)[#ATT] = INITIAL; \
		}\
		nlohmann::json& j = (*this)[#ATT]; \
		for (size_t index = 0; index < j.size(); index++)\
		{\
			j.at(index) = Create##TYPE(#ATT, SUuuid(), j.at(index)); \
		}\
	}\
	std::vector<std::reference_wrapper<std::unique_ptr<TYPE>>> ATT()\
	{\
		std::vector<std::reference_wrapper<std::unique_ptr<TYPE>>> m; \
		nlohmann::json& j = (*this)[#ATT]; \
		for (size_t index; index < j.size(); index++)\
		{\
			m.push_back(Get##TYPE(j.at(index))); \
		}\
		return m; \
	}\
	void ATT##_insert(size_t pos, JUUID uuid)\
	{\
		nlohmann::json placeholder; \
		(*this)[#ATT].insert((*this)[#ATT].begin() + pos, Create##TYPE(#ATT, SUuuid(), placeholder)); \
	}\
	void ATT##_erase(size_t pos)\
	{\
		(*this)[#ATT].erase((*this)[#ATT].begin() + pos); \
	}

#define JPREVIEW(NAME,JEDVALUETYPE)
#define JTRACKUUID(CLASS,NAME,LIMIT,COND)

#endif

#if defined(JEXPOSE_ATT_INIT)

#define JCLASS(CLASS,GETJOBJECTS)
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_MAP_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_VECTOR_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JPREVIEW(NAME,JEDVALUETYPE)
#define JTRACKUUID(CLASS,NAME,LIMIT,COND)

#endif

#if defined(JEXPOSE_ATT_ORDER)

#define JCLASS(CLASS,GETJOBJECTS) static std::vector<std::pair<std::string,JsonToEditorValueType>> Get##CLASS##Attributes()\
{\
	return std::vector<std::pair<std::string,JsonToEditorValueType>>({
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_MAP_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_VECTOR_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JPREVIEW(NAME,JEDVALUETYPE)
#define JTRACKUUID(CLASS,NAME,LIMIT,COND)

#endif

#if defined(JEXPOSE_ATT_FLAGS)

#define JCLASS(CLASS,GETJOBJECTS) enum CLASS##_UpdateFlags\
{
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_MAP_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_VECTOR_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JPREVIEW(NAME,JEDVALUETYPE)
#define JTRACKUUID(CLASS,NAME,LIMIT,COND)

#endif

#if defined(JEXPOSE_ATT_UPDATE)

#define JCLASS(CLASS,GETJOBJECTS) UpdateFlagsMap=\
{
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_MAP_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_VECTOR_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JPREVIEW(NAME,JEDVALUETYPE)
#define JTRACKUUID(CLASS,NAME,LIMIT,COND)

#endif

#if defined(JEXPOSE_ATT_DESTROY)

#define JCLASS(CLASS,GETJOBJECTS)
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) \
	nlohmann::json& j##ATT = (*this)[#ATT];\
	for (nlohmann::json::iterator it = j##ATT.begin(); it != j##ATT.end(); ++it)\
	{\
		Destroy##TYPE(it.value()); \
	}
#define JEXPOSE_VECTOR_OBJECT(TYPE, ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	nlohmann::json& j##ATT = (*this)[#ATT]; \
	for (size_t index = 0; index < j##ATT.size(); index++)\
	{\
		Destroy##TYPE(j##ATT[index]); \
	}

#define JPREVIEW(NAME,JEDVALUETYPE)
#define JTRACKUUID(CLASS,NAME,LIMIT,COND)

#endif