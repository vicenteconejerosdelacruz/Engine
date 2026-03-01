#include "pch.h"
#include "Trigger.h"
#include <Scene.h>
#include <Physics.h>

#if defined(_EDITOR)
namespace Editor
{
	extern void BindRenderableToPickingPass(RenderableID r);
	extern void SelectTrigger(TriggerID trigger);
	extern bool TriggersShouldDraw(SceneUnitId id);
	extern void RegisterTrigger(TriggerID trigger);
	extern void UnRegisterTrigger(TriggerID trigger);
	extern bool TriggersSceneUnitRegistered(SceneUnitId id);
	extern CameraID GetLevelCamera(SceneUnitId id);
}
#endif

namespace Scene
{
	SODEF_FULL(Trigger);

#include <TrackUUID/JDef.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#endif

#if defined(_EDITOR)
	void WriteTriggersJson(SceneUnitId id, nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}
#endif

	Trigger::Trigger(SceneUnitId id, nlohmann::json& json) : SceneObject(id, json)
	{
#include <Attributes/JInit.h>
#include <TriggerAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <TriggerAtt.h>
#include <JEnd.h>
		(*this)["behavior"] = PhysicsBehaviorToString.at(PB_Trigger);
	}

#if defined(_EDITOR)
	void Trigger::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}
#endif

	void Trigger::Initialize()
	{
#include <TrackUUID/JInsert.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void Trigger::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <TriggerAtt.h>
#include <JEnd.h>

		CreatePhysicObject();
	}

	void Trigger::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void Trigger::Destroy()
	{
		if (!physicObject.empty())
		{
			DestroyPhysicObject(physicObject());
		}

#include <Attributes/JDestroy.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void Trigger::CreatePhysicObject()
	{
		nlohmann::json data =
		{
			{ "behavior", "Trigger" },
			{ "geometry", geometry() },
		};

		std::string pOname = name() + "-physicObject";
		physicObject = Physics::CreatePhysicObject(pOname, SUuuid(), data);
		physicObject->CreatePhysicsBehavior();
	}

	void Trigger::visible(bool value)
	{
		at("visible") = value;
#if defined(_EDITOR)
		renderableLines->visible(value);
		renderableShape->visible(value);
#endif
	}

#if defined(_EDITOR)
	nlohmann::json Trigger::CreateRenderableTrigger(std::string name, JUUID uuid, JUUID camId, std::string material)
	{
		PhysicGeometryJsonID pg = geometry();

		bool visible = Editor::TriggersShouldDraw(unit);

		nlohmann::json jrentrigger = nlohmann::json(
			{
				{
					"meshMaterial",
					{
						{ "material", GetMaterialUUIDByName(material) },
						{ "mesh",
							{
								{ "primitive", pg->mesh() }
							}
						}
					}
				},
				{ "castShadows", false },
				{ "shadowed", false },
				{ "name" , name },
				{ "uuid" , uuid },
				{ "position", FromXMFLOAT3(position()) },
				{ "topology", "TRIANGLELIST" },
				{ "rotation" , FromXMFLOAT3(rotation()) },
				{ "scale" , FromXMFLOAT3(scale()) },
				{ "skipMeshes" , {}},
				{ "visible" , visible },
				{ "hidden" , true},
				{ "cameras", { camId }},
				{ "passMaterialOverrides",
					{
						{
							{ "meshIndex", 0 },
							{ "renderPass", GetRenderPassUUIDByName("PickingPass") },
							{ "material", GetMaterialUUIDByName("TriggerPicking") }
						}
					}
				},
				{ "depthStencil",
					{
						{ "BackFace",
							{
								{ "StencilDepthFailOp", "KEEP"},
								{ "StencilFailOp", "KEEP"},
								{ "StencilFunc", "ALWAYS"},
								{ "StencilPassOp", "KEEP" }
							}
						},
						{ "DepthEnable", false },
						{ "DepthFunc", "NONE" },
						{ "DepthWriteMask", "ZERO" },
						{ "FrontFace",
							{
								{ "StencilDepthFailOp", "KEEP"},
								{ "StencilFailOp", "KEEP"},
								{ "StencilFunc", "ALWAYS"},
								{ "StencilPassOp", "KEEP" }
							}
						},
						{ "StencilEnable", false},
						{ "StencilReadMask", 255},
						{ "StencilWriteMask", 255 }
					}
				}
			}
		);
		return jrentrigger;
	}

	void Trigger::CreateRenderableTrigger()
	{
		if (geometry().empty())
			return;

		renderableLines = MAKESUUUID(unit, getUUID());
		renderableShape = MAKESUUUID(unit, getUUID());
		CameraID camera;
#if defined(_EDITOR)
		camera = Editor::GetLevelCamera(unit);
#else
		if (GetCountFromMouseCameras(unit) > 0ULL)
		{
			camera = MAKESUUUID(unit, *GetSwapChainCameras(unit).begin());
		}
#endif
		//CameraID camera = MAKESUUUID(unit, *GetSwapChainCameras(unit).begin());

		nlohmann::json lines = CreateRenderableTrigger(name() + "-lines", renderableLines.uuid(), camera.uuid(), "Translucent_wired");
		nlohmann::json shape = CreateRenderableTrigger(name() + "-shape", renderableShape.uuid(), camera.uuid(), "Translucent");

		shape["renderNext"] = { renderableLines.uuid() };

		nlohmann::json data =
		{
			{ "renderables",
				{
					lines,
					shape
				}
			}
		};

		AttachLevelIntoScene(unit, "triggers", data, [&](SceneUnitId id)
			{
				using namespace Editor;
				BindRenderableToPickingPass(renderableLines);
				BindRenderableToPickingPass(renderableShape);
				renderableLines->OnPick = [&] {Editor::SelectTrigger(SUuuid()); };
				renderableShape->OnPick = [&] {Editor::SelectTrigger(SUuuid()); };
				RegisterTrigger(SUuuid());
			}
		);
	}

	BoundingBox Trigger::GetBoundingBox()
	{
		return BoundingBox(position(), { 0.1f,0.1f,0.1f });
	}

#endif

	void TriggersStep(SceneUnitId unit, float dt)
	{
#if defined(_EDITOR)
		using namespace Editor;
#endif
		auto& Triggers = GetTriggers(unit);
		std::set<TriggerID> tr;
		std::transform(Triggers.begin(), Triggers.end(), std::inserter(tr, tr.begin()), [&](auto o) { return MAKESUUUID(unit, o); });

		std::set<TriggerID> tr2Del;
		std::copy_if(tr.begin(), tr.end(), std::inserter(tr2Del, tr2Del.begin()), [](auto trg)
			{
				return trg->markedForDelete;
			}
		);

		for (auto trg : tr2Del)
		{
#if defined(_EDITOR)
			DeleteRenderable(FROMSUUUID(trg->renderableShape()));
			DeleteRenderable(FROMSUUUID(trg->renderableLines()));
			UnRegisterTrigger(trg);
#endif
			DestroyPhysicObject(trg->physicObject());
			EraseTriggerFromTriggers(FROMSUUUID(trg()));
			DeleteTriggerSceneObject(trg);
		}

#if defined(_EDITOR)
		std::set<TriggerID> trCreateRenderables;
		std::copy_if(tr.begin(), tr.end(), std::inserter(trCreateRenderables, trCreateRenderables.begin()), [](auto trg)
			{
				return trg->renderableShape.empty();
			}
		);

		std::set<TriggerID> trTransformation;
		std::copy_if(tr.begin(), tr.end(), std::inserter(trTransformation, trTransformation.begin()), [](auto trg)
			{
				return !trg->renderableShape.empty() && (
					trg->dirty(Trigger::Update_position) ||
					trg->dirty(Trigger::Update_rotation) ||
					trg->dirty(Trigger::Update_scale)
					);
			}
		);

		std::set<TriggerID> trColor;
		std::copy_if(tr.begin(), tr.end(), std::inserter(trColor, trColor.begin()), [](auto trg)
			{
				return !trg->renderableShape.empty() && trg->dirty(Trigger::Update_color);
			}
		);

		if (TriggersSceneUnitRegistered(unit))
		{
			for (auto trg : trCreateRenderables)
			{
				trg->CreateRenderableTrigger();
			}
		}

		for (auto trg : trTransformation)
		{
			trg->renderableShape->position(trg->position());
			trg->renderableShape->rotation(trg->rotation());
			trg->renderableShape->scale(trg->scale());
			trg->renderableLines->position(trg->position());
			trg->renderableLines->rotation(trg->rotation());
			trg->renderableLines->scale(trg->scale());
			if (trg->dirty(Trigger::Update_scale))
			{
				trg->physicObject->DestroyPhisicsBehavior();
				trg->physicObject->CreatePhysicsBehavior();
			}
			else
			{
				trg->physicObject->UpdateGlobalPoseFromTrigger();
			}
			trg->clean(Trigger::Update_position);
			trg->clean(Trigger::Update_rotation);
			trg->clean(Trigger::Update_scale);
		}

		for (auto trg : trColor)
		{
			XMFLOAT4 color = trg->color();
			XMFLOAT3 baseColor = { color.x,color.y,color.z };
			XMFLOAT3 lineBaseColor = baseColor * 1.3f;
			float alpha = color.w;
			for (unsigned int i = 0; i < Renderer::numFrames; i++)
			{
				trg->renderableShape->WriteConstantsBuffer("baseColor", baseColor, i);
				trg->renderableShape->WriteConstantsBuffer("alpha", alpha, i);
				trg->renderableLines->WriteConstantsBuffer("baseColor", lineBaseColor, i);
			}
			trg->clean(Trigger::Update_color);
		}
#endif
	}

	void DestroyTriggers()
	{
		for (auto& [id, container] : TriggerSUsceneObjects)
		{
			for (auto& [uuid, _] : container)
			{
				TriggerID t = MAKESUUUID(id, uuid);
				DeleteTriggerSceneObject(t);
			}
		}
#include <TrackUUID/JClear.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void DestroyTriggers(SceneUnitId id)
	{
		std::set<JUUID> uuids;
		std::transform(TriggerSUsceneObjects.at(id).begin(), TriggerSUsceneObjects.at(id).end(), std::inserter(uuids, uuids.begin()), [](auto& pair) { return pair.first; });
		for (auto& uuid : uuids)
		{
			DeleteTriggerSceneObject(MAKESUUUID(id, uuid));
		}
#include <TrackUUID/JClearUnit.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void DeleteTrigger(SceneUnitId id, JUUID uuid)
	{
		TriggerID tg = MAKESUUUID(id, uuid);
		tg->markedForDelete = true;
	}
}