#pragma once
#include <Controller.h>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>

namespace Game
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <BrawlerSceneControllerAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <BrawlerSceneControllerAtt.h>
#include <JEnd.h>

#endif

	struct BrawlerSceneController : Controller
	{
#include <Attributes/JFlags.h>
#include <BrawlerSceneControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <BrawlerSceneControllerAtt.h>
#include <JEnd.h>

		//Constructor and Binding
		BrawlerSceneController(nlohmann::json& json);
		virtual void SetInitialConditions();
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		DECL_CONTROLLER_DRAWER(BrawlerSceneController, Controller);
#endif
		virtual void Map(SUUUID so);
		virtual void Unmap();

		//UI
		void CreateVenomUI(SceneUnitId id);
		void UpdateVenomUI(SceneUnitId id);

		//Step
		virtual void Step(float delta);
		//Rendering
		virtual void Render(SceneUnitId id);

		HtmlUIInstanceID venomUIInstance;
	};
};