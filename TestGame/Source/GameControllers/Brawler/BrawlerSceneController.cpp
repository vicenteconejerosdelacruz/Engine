#include "pch.h"
#include "BrawlerSceneController.h"
#if defined(_EDITOR)
#include <Editor.h>
#endif

namespace Game
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <BrawlerSceneControllerAtt.h>
#include <JEnd.h>

#endif

	//Constructor and Binding
	BrawlerSceneController::BrawlerSceneController(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <BrawlerSceneControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <BrawlerSceneControllerAtt.h>
#include <JEnd.h>
		SetInitialConditions();
	}

	void BrawlerSceneController::SetInitialConditions()
	{
	}

#if defined(_EDITOR)
	void BrawlerSceneController::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <BrawlerSceneControllerAtt.h>
#include <JEnd.h>
		j.erase("uuid");
	}
#endif

	void BrawlerSceneController::Map(SUUUID so)
	{
		using namespace Scene;
		Controller::Map(so);


		SetInitialConditions();
	}

	void BrawlerSceneController::Unmap()
	{
		Controller::Unmap();
	}

	//UI
	void BrawlerSceneController::CreateVenomUI(SceneUnitId id)
	{
		venomUIInstance = venomUI() + "-" + getUUID();
		CreateHtmlUIInstance(venomUIInstance(), [&]()
			{
				return std::make_unique<HtmlUIInstance>(id, venomUIInstance(), venomUI());
			}
		);
	}

	void BrawlerSceneController::UpdateVenomUI(SceneUnitId id)
	{
		venomUIInstance->UpdateTexture(id);
		venomUIInstance->Resolve(id);
	}

	//Step
	void BrawlerSceneController::Step(float delta)
	{
	}

	//Rendering
	void BrawlerSceneController::Render(SceneUnitId id)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(id) || Editor::IsPaused(id))
			return;
#endif
		venomUIInstance.empty() ? CreateVenomUI(id) : UpdateVenomUI(id);
	}
};