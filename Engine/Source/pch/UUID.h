#pragma once

#include <string>
#include <tuple>
#include "nlohmann/json.hpp"

typedef std::string JUUID;
typedef std::string JNAME;
typedef std::tuple<JUUID, JNAME> JUUIDName;
typedef std::tuple<SceneUnitId, JUUID> SUUUID;
#define MAKESUUUID(id,juuid) std::make_tuple(id,juuid)

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
	JUUID operator()()
	{
		return uuid;
	}
	JUUID operator+()
	{
		return uuid;
	}
	bool empty()
	{
		return uuid.empty();
	}
	void clear()
	{
		uuid = "";
	}
};

#define DEF_UUID_TYPE(NAMESPACE,TYPE,GETTER)\
namespace NAMESPACE\
{\
	struct TYPE;\
	extern std::unique_ptr<TYPE>& GETTER(JUUID uuid);\
};\
typedef TUUID<NAMESPACE::TYPE, NAMESPACE::GETTER> TYPE##UUID;\
template <>\
struct std::hash<TYPE##UUID>\
{\
	std::size_t operator()(const TYPE##UUID& other) const\
	{\
		return std::hash<std::string>{}(other.uuid);\
	}\
};

template<typename T, std::unique_ptr<T>& F(SceneUnitId, JUUID)>
struct TSUUUID {
	SUUUID SUuuid;
	TSUUUID() {}
	TSUUUID(const TSUUUID& other)
	{
		SUuuid = other.SUuuid;
	}
	TSUUUID(const SUUUID& nSUuuid)
	{
		SUuuid = nSUuuid;
	}
	TSUUUID operator=(TSUUUID other)
	{
		SUuuid = other.SUuuid;
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
	bool operator<(const TSUUUID& other) const
	{
		//return (std::get<0>(SUuuid) < std::get<0>(other.SUuuid)) && (std::get<1>(SUuuid) < std::get<1>(other.SUuuid));
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
	}
	SceneUnitId unit()
	{
		SceneUnitId& id = std::get<0>(SUuuid);
		return id;
	}
	JUUID uuid()
	{
		JUUID& uuid = std::get<1>(SUuuid);
		return uuid;
	}
};

#define DEF_SUUUID_TYPE(NAMESPACE,TYPE,GETTER)\
namespace NAMESPACE\
{\
	struct TYPE;\
	extern std::unique_ptr<TYPE>& GETTER(SceneUnitId, JUUID uuid);\
};\
typedef TSUUUID<NAMESPACE::TYPE, NAMESPACE::GETTER> TYPE##SUUUID;\
template <>\
struct std::hash<TYPE##SUUUID>\
{\
	std::size_t operator()(const TYPE##SUUUID& other) const\
	{\
		size_t hash = 0;\
		nostd::hash_combine(hash, std::get<0>(other.SUuuid), std::get<1>(other.SUuuid));\
		return hash;\
	}\
};

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

