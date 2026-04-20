#include "pch.h"
#include "BrawlerCamera.h"
#include "../Characters/Heroes/Venom.h"
#include "../Scene/BrawlerScene.h"
#if defined(_EDITOR)
#include <Editor.h>
#endif

namespace Game::Brawler
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include <Brawler/BrawlerCameraAtt.h>
#include <JEnd.h>
#endif

	BrawlerCamera::BrawlerCamera(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <Brawler/BrawlerCameraAtt.h>
#include <JEnd.h>
#include <Attributes/JUpdate.h>
#include <Brawler/BrawlerCameraAtt.h>
#include <JEnd.h>
		initialFollowX = followX();
		initialFollowY = followY();
	}

	void BrawlerCamera::SetInitialConditions()
	{
		followX(initialFollowX);
		followY(initialFollowY);
	}

#if defined(_EDITOR)
	void BrawlerCamera::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <Brawler/BrawlerCameraAtt.h>
#include <JEnd.h>
		Controller::WriteJson(j);
	}
#endif

	void BrawlerCamera::Map(SUUUID so)
	{
		Controller::Map(so);

		camera = so;
		venomR = MAKESUUUID(std::get<0>(so), venom());
		YcamInitial = camera->position().y;
		Ycam2venom = YcamInitial - venomR->position().y;
		GetController<Brawler::BrawlerScene>(unit, sceneController())->RegisterCamera(uuid());

		SetInitialConditions();
	}

	void BrawlerCamera::Unmap()
	{
		Controller::Unmap();
		camera.clear();
		venomR.clear();
	}

	void BrawlerCamera::Step(float delta)
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
	v8_templates_creators BrawlerCamera::GetV8TemplatesCreators()
	{
		v8_templates_creators creators = Controller::GetV8TemplatesCreators();
#include <Attributes/JV8Templates.h>
#include <Brawler/BrawlerCameraAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_context_creators BrawlerCamera::GetV8ContextCreators()
	{
		v8_context_creators creators = Controller::GetV8ContextCreators();
#include <Attributes/JV8Context.h>
#include <Brawler/BrawlerCameraAtt.h>
#include <JEnd.h>
		return creators;
	}

	v8_functions_creators BrawlerCamera::GetV8FunctionsCreators()
	{
		return {
			//{ "StopCameraFollow", v8_wrap_call([&] { follow = false; }) },
			//{ "StartCameraFollow", v8_wrap_call([&] { follow = true; }) },
		};
	}
}

