#pragma once
#include <Controller.h>

namespace Game
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <BrawlerCameraControllerAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <BrawlerCameraControllerAtt.h>
#include <JEnd.h>

#endif

	struct BrawlerCameraController : Controller
	{
#include <Attributes/JFlags.h>
#include <BrawlerCameraControllerAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <BrawlerCameraControllerAtt.h>
#include <JEnd.h>

		BrawlerCameraController(nlohmann::json& json);
		virtual void SetInitialConditions();
#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
		DECL_CONTROLLER_DRAWER(BrawlerCameraController, Controller);
#endif

		virtual void Map(SUUUID so);
		virtual void Unmap();
		virtual void Step(float delta);

		//JS binding
		virtual v8_templates_creators GetV8TemplatesCreators();
		virtual v8_context_creators GetV8ContextCreators();
		virtual v8_functions_creators GetV8FunctionsCreators();

		//object to interact with
		CameraID camera;
		RenderableID venomR;
		float Ycam2venom;
		float YcamInitial;
	};
}