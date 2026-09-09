#pragma once
#include <ImGuizmo.h>
#include <DirectXMath.h>
#include <map>
#include <SceneObject.h>

struct GizmoInteraction
{
	GizmoInteraction()
	{
		gizmoOperation = ImGuizmo::TRANSLATE;
		gizmoMode = ImGuizmo::WORLD;
		gizmoCentroidMx = XMFLOAT4X4();
		gizmoApplyOp = false;
		soRotation = std::map<SceneObject*, XMFLOAT3>();
		soScale = std::map<SceneObject*, XMFLOAT3>();
		so2bb = std::map<SceneObject*, XMFLOAT3>();
		bb2gizmo = std::map<SceneObject*, XMFLOAT3>();
		gizmoRotation = XMFLOAT3();
		gizmoPosition = XMFLOAT3();
		gizmoScale = XMFLOAT3();
	}

	ImGuizmo::OPERATION gizmoOperation;//(ImGuizmo::TRANSLATE);
	ImGuizmo::MODE gizmoMode;// (ImGuizmo::WORLD);
	XMFLOAT4X4 gizmoCentroidMx;
	bool gizmoApplyOp;// = false;
	std::map<SceneObject*, XMFLOAT3> soRotation;
	std::map<SceneObject*, XMFLOAT3> soScale;
	std::map<SceneObject*, XMFLOAT3> so2bb;
	std::map<SceneObject*, XMFLOAT3> bb2gizmo;
	XMFLOAT3 gizmoRotation;
	XMFLOAT3 gizmoPosition;
	XMFLOAT3 gizmoScale;
};