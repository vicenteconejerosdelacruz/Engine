#pragma once

/*
inline void UpdateSystemCreatedFromJson(unsigned int& flags, nlohmann::json object) {
	flags |= (object.contains("systemCreated") and object["systemCreated"] != "") ? TemplateFlags::SystemCreated : TemplateFlags::None;
}
*/
inline DirectX::XMFLOAT3 JsonToFloat3(nlohmann::json f3) {
	return DirectX::XMFLOAT3({ f3[0], f3[1], f3[2] });
}

inline void JsonToFloat3(DirectX::XMFLOAT3& value, nlohmann::json f3, std::string key) {
	if (f3.contains(key)) { value = XMFLOAT3({ f3[key][0], f3[key][1], f3[key][2] }); }
}

template<typename T>
inline void SetIfMissingJson(nlohmann::json& j, std::string key, T value)
{
	if (!j.contains(key)) { j[key] = value; }
}

template<>
inline void SetIfMissingJson<DirectX::XMFLOAT3>(nlohmann::json& j, std::string key, DirectX::XMFLOAT3 value)
{
	if (!j.contains(key)) { j[key] = nlohmann::json::array({ value.x,value.y,value.z }); }
}

template<>
inline void SetIfMissingJson<DirectX::XMFLOAT2>(nlohmann::json& j, std::string key, DirectX::XMFLOAT2 value)
{
	if (!j.contains(key)) { j[key] = nlohmann::json::array({ value.x,value.y }); }
}

inline DirectX::XMFLOAT2 JsonToFloat2(nlohmann::json f2) {
	return DirectX::XMFLOAT2({ f2[0], f2[1] });
}

inline void JsonToFloat2(DirectX::XMFLOAT2& value, nlohmann::json f2, std::string key) {
	if (f2.contains(key)) { value = XMFLOAT2({ f2[key][0], f2[key][1] }); }
}

template<typename T>
std::set<T> TransformJsonArrayToSet(nlohmann::json j) {
	std::set<T> set;
	for (auto& value : j) {
		T v = value;
		set.insert(v);
	}
	return set;
}

template<typename T>
void TransformJsonArrayToSet(std::set<T>& set, nlohmann::json j, std::string key) {
	if (!j.contains(key)) return;
	for (auto& value : j[key])
	{
		T v = value;
		set.insert(v);
	}
}

template<typename T>
void ReplaceFromJson(T& value, nlohmann::json object, const std::string& key) {
	if (object.contains(key)) value = static_cast<T>(object[key]);
}

inline void RemoveJsonAttributes(nlohmann::json& dst, std::vector<std::string>& atts) {
	for (auto& key : atts)
	{
		if (dst.contains(key)) dst.erase(key);
	}
}

