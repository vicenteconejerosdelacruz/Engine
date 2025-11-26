#pragma once

#include <string>
#include <tuple>
#include "nlohmann/json.hpp"

typedef std::string JUUID;
typedef std::string JNAME;
typedef std::tuple<JUUID, JNAME> JUUIDName;

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

//typedef std::unordered_map<TYPE##UUID, std::unique_ptr<TYPE>, TYPE##UUID::Hash> TYPE##UUID##Map;

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

inline void SortUUIDByName(std::vector<JUUIDName>& uuidNames)
{
	std::sort(uuidNames.begin(), uuidNames.end(), [](JUUIDName a, JUUIDName b)
		{
			return std::get<1>(a) < std::get<1>(b);
		}
	);
}

inline JNAME GetName(std::string uuid, auto getContainedObjects)
{
	auto& containedObjects = getContainedObjects();
	return containedObjects.contains(uuid) ? std::get<0>(containedObjects.at(uuid)) : "";
}

inline JUUID GetUUIDByName(std::string name, auto getContainedObjects)
{
	auto& containedObjects = getContainedObjects();
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

