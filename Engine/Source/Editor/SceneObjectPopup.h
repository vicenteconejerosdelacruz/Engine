#pragma once

struct SceneObjectPopup
{
	void Draw();

	bool show = false;
	ImVec2 pos;
	std::string name;
	SceneUnitId id;
	JUUID uuid;
	SceneObjectType type;
};