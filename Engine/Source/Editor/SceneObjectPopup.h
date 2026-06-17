#pragma once

struct SceneObjectPopup
{
	void Draw();

	bool show = false;
	int openedCollapsableItem;
	bool moldTreeSelection;
	ImVec2 pos;
	std::string name;
	SceneUnitId id;
	JUUID uuid;
	SceneObjectType type;
	std::set<std::string> selected_uuids;
};