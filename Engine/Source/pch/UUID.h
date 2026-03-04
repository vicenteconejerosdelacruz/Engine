#ifndef _UUID_TYPES_H
#define _UUID_TYPES_H

#include <string>
#include <tuple>
#include "nlohmann/json.hpp"

using JUUID = std::string;
using JNAME = std::string;
using JUUIDName = std::tuple<JUUID, JNAME>;
using SUUUID = std::tuple<SceneUnitId, JUUID>;

#define MAKESUUUID(id,juuid) std::make_tuple(id,juuid)
#define FROMSUUUID(tup) std::get<0>(tup),std::get<1>(tup)

template<typename T, std::unique_ptr<T>& F(JUUID)>
struct TUUID {
	JUUID uuid;
	TUUID() {}
	TUUID(const TUUID& other)
	{
		uuid = other.uuid;
	}
	TUUID(const JUUID& nuuid)
	{
		uuid = nuuid;
	}
	TUUID operator=(TUUID other)
	{
		uuid = other.uuid;
		return *this;
	}
	TUUID operator=(JUUID nuuid)
	{
		uuid = nuuid;
		return *this;
	}
	bool operator==(const TUUID& other) const
	{
		return other.uuid == uuid;
	}
	std::unique_ptr<T>& operator->()
	{
		return F(uuid);
	}
	std::unique_ptr<T>& operator->() const
	{
		return F(uuid);
	}
	std::unique_ptr<T>& operator*()
	{
		return F(uuid);
	}
	bool operator<(const TUUID& other) const
	{
		return uuid < other.uuid;
	}
	JUUID operator()() const
	{
		return uuid;
	}
	JUUID operator()()
	{
		return uuid;
	}
	bool operator!() const
	{
		return empty();
	}
	bool operator!()
	{
		return empty();
	}
	explicit operator bool() const
	{
		return !empty();
	}
	explicit operator bool()
	{
		return !empty();
	}
	JUUID operator+()
	{
		return uuid;
	}
	bool empty() const
	{
		return uuid.empty();
	}
	bool empty()
	{
		return uuid.empty();
	}
	void clear()
	{
		uuid = "";
	}
	void replace(JUUID u)
	{
		uuid = u;
	}
};

#define DEF_TEMPLATE_ID(TYPE,GETTER)\
using TYPE##ID = TUUID<TYPE, GETTER>;

#define DEF_TEMPLATE_ID_DEP(TYPE,GETTER)\
struct TYPE;\
extern std::unique_ptr<TYPE>& GETTER(JUUID uuid);\
DEF_TEMPLATE_ID(TYPE,GETTER)

#define DEF_TEMPLATE_ID_HASH(TYPE)\
template <>\
struct std::hash<TYPE##ID>\
{\
	std::size_t operator()(const TYPE##ID& other) const\
	{\
		return std::hash<std::string>{}(other.uuid);\
	}\
}

template<typename T, std::unique_ptr<T>& F(SceneUnitId, JUUID)>
struct TSUUUID {
	SUUUID SUuuid;
	std::function<bool()> validator = nullptr;
	TSUUUID() {}
	TSUUUID(const TSUUUID& other)
	{
		SUuuid = other.SUuuid;
		validator = other.validator;
	}
	TSUUUID(const TSUUUID& other, std::function<bool()> validator)
	{
		SUuuid = other.SUuuid;
		this->validator = validator;
	}
	TSUUUID(const SUUUID& nSUuuid)
	{
		SUuuid = nSUuuid;
	}
	TSUUUID(const SUUUID& nSUuuid, std::function<bool()> validator)
	{
		SUuuid = nSUuuid;
		this->validator = validator;
	}
	TSUUUID operator=(TSUUUID other)
	{
		SUuuid = other.SUuuid;
		validator = other.validator;
		return *this;
	}
	TSUUUID operator=(SUUUID nSUuuid)
	{
		SUuuid = nSUuuid;
		return *this;
	}
	bool operator==(const TSUUUID& other) const
	{
		return (std::get<0>(other.SUuuid) == std::get<0>(SUuuid)) && (std::get<1>(other.SUuuid) == std::get<1>(SUuuid));
	}
	std::unique_ptr<T>& operator->()
	{
		return F(std::get<0>(SUuuid), std::get<1>(SUuuid));
	}
	std::unique_ptr<T>& operator->() const
	{
		return F(std::get<0>(SUuuid), std::get<1>(SUuuid));
	}
	std::unique_ptr<T>& operator*()
	{
		return F(std::get<0>(SUuuid), std::get<1>(SUuuid));
	}
	std::unique_ptr<T>& operator*() const
	{
		return F(std::get<0>(SUuuid), std::get<1>(SUuuid));
	}
	bool operator<(const TSUUUID& other) const
	{
		if (std::get<0>(SUuuid) != std::get<0>(other.SUuuid))
		{
			return std::get<0>(SUuuid) < std::get<0>(other.SUuuid);
		}
		return std::get<1>(SUuuid) < std::get<1>(other.SUuuid);
	}
	SUUUID operator()()
	{
		return SUuuid;
	}
	SUUUID operator+()
	{
		return SUuuid;
	}
	bool operator!() const
	{
		return empty();
	}
	bool operator!()
	{
		return empty();
	}
	explicit operator bool() const
	{
		return validator ? (validator() && !empty()) : !empty();
	}
	explicit operator bool()
	{
		return validator ? (validator() && !empty()) : !empty();
	}
	bool empty()
	{
		return std::get<1>(SUuuid).empty();
	}
	void clear()
	{
		SceneUnitId& unit = std::get<0>(SUuuid);
		JUUID& uuid = std::get<1>(SUuuid);
		unit = 0;
		uuid = "";
		validator = nullptr;
	}
	SceneUnitId unit() const
	{
		const SceneUnitId& id = std::get<0>(SUuuid);
		return id;
	}
	SceneUnitId unit()
	{
		SceneUnitId& id = std::get<0>(SUuuid);
		return id;
	}
	JUUID uuid() const
	{
		return std::get<1>(SUuuid);
	}
	JUUID uuid()
	{
		return std::get<1>(SUuuid);
	}
};

#define DEF_SCENEOBJECT_ID(TYPE)\
using TYPE##ID = TSUUUID<TYPE, Get##TYPE##SceneObject>;

#define DEF_SCENEOBJECT_ID_DEP(TYPE)\
struct TYPE;\
extern std::unique_ptr<TYPE>& Get##TYPE##SceneObject(SceneUnitId, JUUID uuid);\
DEF_SCENEOBJECT_ID(TYPE)

#define DEF_SCENEOBJECT_ID_HASH(TYPE)\
template <>\
struct std::hash<TYPE##ID>\
{\
	std::size_t operator()(const TYPE##ID& other) const\
	{\
		size_t hash = 0;\
		nostd::hash_combine(hash, std::get<0>(other.SUuuid), std::get<1>(other.SUuuid));\
		return hash;\
	}\
}

//Create a new UUID
inline std::string getUUID()
{
	UUID uuid = { 0 };
	std::string guid;

	// Create uuid or load from a string by UuidFromString() function
	::UuidCreate(&uuid);

	// If you want to convert uuid to string, use UuidToString() function
	RPC_CSTR szUuid = NULL;
	if (::UuidToStringA(&uuid, &szUuid) == RPC_S_OK)
	{
		guid = (char*)szUuid;
		::RpcStringFreeA(&szUuid);
	}

	return guid;
}

inline int FindSelectableIndex(auto selectables, nlohmann::json& json, auto att)
{
	auto& value = json.at(att);
	return static_cast<int>(std::find_if(selectables.begin(), selectables.end(), [value](JUUIDName uuidName)
		{
			return value == std::get<0>(uuidName);
		}
	) - selectables.begin());
}

inline std::function<std::vector<JUUIDName>()> SortUUIDNameByName(std::function<std::vector<JUUIDName>()> getUUIDNames)
{
	return [getUUIDNames]()
		{
			std::vector<JUUIDName> uuidNames = getUUIDNames();
			std::sort(uuidNames.begin(), uuidNames.end(), [](JUUIDName a, JUUIDName b)
				{
					return std::get<1>(a) < std::get<1>(b);
				}
			);
			return uuidNames;
		};
};

inline std::function<std::vector<JUUIDName>()> SortUUIDSUNameByName(SceneUnitId id, std::function<std::vector<JUUIDName>(SceneUnitId)> getUUIDNames)
{
	return [getUUIDNames, id]()
		{
			std::vector<JUUIDName> uuidNames = getUUIDNames(id);
			std::sort(uuidNames.begin(), uuidNames.end(), [](JUUIDName a, JUUIDName b)
				{
					return std::get<1>(a) < std::get<1>(b);
				}
			);
			return uuidNames;
		};
};

inline void SortUUIDByName(std::vector<JUUIDName>& uuidNames)
{
	std::sort(uuidNames.begin(), uuidNames.end(), [](JUUIDName a, JUUIDName b)
		{
			return std::get<1>(a) < std::get<1>(b);
		}
	);
}

inline JNAME GetName(JUUID uuid, auto getContainedObjects)
{
	auto& containedObjects = getContainedObjects();
	return containedObjects.contains(uuid) ? std::get<0>(containedObjects.at(uuid)) : "";
}

inline JNAME GetSUName(SceneUnitId id, JUUID uuid, auto getContainedObjects)
{
	auto& containedObjects = getContainedObjects(id);
	return containedObjects.contains(uuid) ? std::get<0>(containedObjects.at(uuid)) : "";
}

inline JUUID GetUUIDByName(JNAME name, auto getContainedObjects)
{
	auto& containedObjects = getContainedObjects();
	for (auto& [uuid, T] : containedObjects)
	{
		if (std::get<0>(T) == name) return uuid;
	}

	return "";
}

inline JUUID GetUUIDBySUName(SceneUnitId id, JNAME name, auto getContainedObjects)
{
	auto& containedObjects = getContainedObjects(id);
	for (auto& [uuid, T] : containedObjects)
	{
		if (std::get<0>(T) == name) return uuid;
	}

	return "";
}

inline std::vector<JNAME> GetNames(auto& items)
{
	std::vector<JNAME> names;
	std::transform(items.begin(), items.end(), std::back_inserter(names), [](auto& pair)
		{
			return std::get<0>(pair.second);
		}
	);
	std::sort(names.begin(), names.end());
	return names;
}

inline std::vector<JUUIDName> GetUUIDsNames(auto& items)
{
	std::vector<JUUIDName> uuidsNames;
	std::transform(items.begin(), items.end(), std::back_inserter(uuidsNames), [](auto& pair)
		{
			return std::make_tuple(pair.first, std::get<0>(pair.second));
		}
	);
	SortUUIDByName(uuidsNames);
	return uuidsNames;
}

#endif