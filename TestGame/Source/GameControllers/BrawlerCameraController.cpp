#include "pch.h"
#include "BrawlerCameraController.h"
#if defined(_EDITOR)
#include <Editor.h>
#endif

namespace Game
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <BrawlerCameraControllerAtt.h>
#include <JEnd.h>

#endif

	BrawlerCameraController::BrawlerCameraController(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <BrawlerCameraControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <BrawlerCameraControllerAtt.h>
#include <JEnd.h>
	}

	void BrawlerCameraController::SetInitialConditions()
	{
	}

#if defined(_EDITOR)
	void BrawlerCameraController::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <BrawlerCameraControllerAtt.h>
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
		p.x = venomR->position().x;
		camera->position(p);
	}
}

