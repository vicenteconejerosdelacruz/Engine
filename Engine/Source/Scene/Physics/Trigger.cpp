#include "pch.h"
#include "Trigger.h"
#include <Scene.h>
#include <Physics.h>

#if defined(_EDITOR)
namespace Editor
{
	extern void BindRenderableToPickingPass(RenderableID r);
	extern void SelectTrigger(TriggerID trigger);
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

#if defined(_EDITOR)
	nlohmann::json Trigger::CreateRenderableTrigger(std::string name, JUUID uuid, JUUID camId, std::string material)
	{
		PhysicGeometryJsonID pg = geometry();

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
				{ "position" , { 0.0f, 0.0f, 0.0f} },
				{ "topology", "TRIANGLELIST" },
				{ "rotation" , { 0.0, 0.0, 0.0 } },
				{ "scale" , { 1.0f, 1.0f, 1.0f } },
				{ "skipMeshes" , {}},
				{ "visible" , true},
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
		CameraID camera = MAKESUUUID(unit, *GetSwapChainCameras(unit).begin());

		nlohmann::json lines = CreateRenderableTrigger(name() + "-lines", renderableLines.uuid(), camera.uuid(), "Translucent");
		nlohmann::json shape = CreateRenderableTrigger(name() + "-shape", renderableShape.uuid(), camera.uuid(), "Translucent_wired");

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

		for (auto trg : trCreateRenderables)
		{
			trg->CreateRenderableTrigger();
		}

		for (auto trg : trTransformation)
		{
			trg->renderableShape->position(trg->position());
			trg->renderableShape->rotation(trg->rotation());
			trg->renderableShape->scale(trg->scale());
			trg->renderableLines->position(trg->position());;
			trg->renderableLines->rotation(trg->rotation());;
			trg->renderableLines->scale(trg->scale());;
			trg->clean(Trigger::Update_position);
			trg->clean(Trigger::Update_rotation);
			trg->clean(Trigger::Update_scale);
		}
#endif
	}

	void DestroyTriggers()
	{
#include <TrackUUID/JClear.h>
#include <TriggerAtt.h>
#include <JEnd.h>
	}

	void DestroyTrigger(SceneUnitId id)
	{
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