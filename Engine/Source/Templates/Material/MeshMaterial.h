#pragma once

struct MeshMaterial
{
	//std::string mesh;
	nlohmann::json mesh;
	std::string materialUUID;
};

inline MeshMaterial ToMeshMaterial(nlohmann::json j)
{
	return { j.at("mesh"), j.at("material") };
}
inline nlohmann::json FromMeshMaterial(MeshMaterial mm)
{
	return {
		{"mesh",mm.mesh},
		{"material",mm.materialUUID}
	};
}
