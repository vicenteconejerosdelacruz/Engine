#include "pch.h"
#include "BrawlerCameraController.h"
#include "../Heroes/VenomController.h"
#include "../Scene/BrawlerSceneController.h"
#if defined(_EDITOR)
#include <Editor.h>
#endif

namespace Game
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <Brawler/BrawlerCameraControllerAtt.h>
#include <JEnd.h>

#endif

	BrawlerCameraController::BrawlerCameraController(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <Brawler/BrawlerCameraControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <Brawler/BrawlerCameraControllerAtt.h>
#include <JEnd.h>
	}

	void BrawlerCameraController::SetInitialConditions()
	{
	}

#if defined(_EDITOR)
	void BrawlerCameraController::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <Brawler/BrawlerCameraControllerAtt.h>
#include <JEnd.h>

		Controller::WriteJson(j);
		j.erase("uuid");
	}
#endif

	void BrawlerCameraController::Map(SUUUID so)
	{
		Controller::Map(so);

		camera = so;
		venomR = MAKESUUUID(std::get<0>(so), venom());
		YcamInitial = camera->position().y;
		Ycam2venom = YcamInitial - venomR->position().y;
		GetController<BrawlerSceneController>(unit, sceneController())->RegisterCamera(controller);
	}

	void BrawlerCameraController::Unmap()
	{
		Controller::Unmap();
		camera.clear();
		venomR.clear();
	}

	void BrawlerCameraController::Step(float delta)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
			return;
#endif

		XMFLOAT3 p = camera->position();

		if (followX())
		{
			p.x = venomR->position().x;
		}

		if (followY())
		{
			p.y = venomR->position().y + Ycam2venom;
		}
		else
		{
			p.y = YcamInitial;
		}

		camera->position(p);
	}

	//JS binding
	v8_templates_creators BrawlerCameraController::GetV8TemplatesCreators()
	{
		v8_templates_creators creators = Controller::GetV8TemplatesCreators();
#include <Attributes/JV8Templates.h>
#include <Brawler/BrawlerCameraControllerAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_context_creators BrawlerCameraController::GetV8ContextCreators()
	{
		v8_context_creators creators = Controller::GetV8ContextCreators();
#include <Attributes/JV8Context.h>
#include <Brawler/BrawlerCameraControllerAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_functions_creators BrawlerCameraController::GetV8FunctionsCreators()
	{
		return {
			//{ "StopCameraFollow", v8_wrap_call([&] { follow = false; }) },
			//{ "StartCameraFollow", v8_wrap_call([&] { follow = true; }) },
		};
	}
}

