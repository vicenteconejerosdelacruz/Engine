#include "pch.h"
#include "PumpkinBomb.h"
#include <Scene.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

extern DX::StepTimer timer;

namespace Game::Brawler
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include "PumpkinBombAtt.h"
#include <JEnd.h>
#endif

	PumpkinBomb::PumpkinBomb(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include "PumpkinBombAtt.h"
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include "PumpkinBombAtt.h"
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include "PumpkinBombAtt.h"
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void PumpkinBomb::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include "PumpkinBombAtt.h"
#include <JEnd.h>
		Controller::WriteJson(j);
	}
#endif

	void PumpkinBomb::Map(SUUUID so)
	{
		Controller::Map(so);

		SceneObjectType type = GetSceneObjectType(so);
		if (type == SO_Renderables)
		{
			renderable = so;
		}
		if (renderable && renderable->at("physicObject").size() > 0)
		{
			physicObject = renderable->at("physicObject").at(0);
			RegisterContactCallback(PB_Static, physicObject(), [&](JUUID uuid, unsigned int event)
				{
					OnStaticContactEvent(uuid, event);
				}
			);
		}
	}

	void PumpkinBomb::Unmap()
	{
		Controller::Unmap();
	}

	void PumpkinBomb::Step(float delta)
	{

	}
	void PumpkinBomb::OnStaticContactEvent(JUUID phO, unsigned int event)
	{
		XMFLOAT3 pos = renderable->position() + explosion_offset();
		auto cameras = renderable->cameras();
		ControllerBinding en = enemy();

		Scene::CreateSceneObjectFromMold(unit, explosion(),
			[pos, cameras, en](SceneObjectType type, nlohmann::json json, std::string name)
			{
				if (type == SO_Triggers && en)
				{
					return nlohmann::json(
						{
							{ "name", std::string(json.at("name")) },
							//{ "uuid", getUUID() },
							{ "position", FromXMFLOAT3(pos) },
							{ "cameras", cameras },
							{ "bindings", {
								{
									{ "bindingType", BindingTypeToString.at(BT_Controller) },
									{ "controllerName", "greengoblin" },
									{ "physicObjectIndex", 0 },
									{ "bindingName", "greengoblin" },
									{ "uuid", en.uuid }
								}
							}
							}
						}
					);
				}
				else
				{
					return nlohmann::json(
						{
							//{ "name", std::string(json.at("name")) },
							//{ "uuid", getUUID() },
							{ "position", FromXMFLOAT3(pos) },
							{ "cameras", cameras },
						}
						);
				}
			}
		);
		renderable->markedForDelete = true;
	}
}
