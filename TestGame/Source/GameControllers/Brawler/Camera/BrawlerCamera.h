#pragma once
#include <Controller.h>

namespace Game
{
	namespace Brawler
	{
#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include <Brawler/BrawlerCameraAtt.h>
#include <JEnd.h>
#include <Editor/JDrawersDecl.h>
#include <Brawler/BrawlerCameraAtt.h>
#include <JEnd.h>
#endif

		struct BrawlerCamera : Controller
		{
#include <Attributes/JFlags.h>
#include <Brawler/BrawlerCameraAtt.h>
#include <JEnd.h>
#include <Attributes/JDecl.h>
#include <Brawler/BrawlerCameraAtt.h>
#include <JEnd.h>

			BrawlerCamera(nlohmann::json& json);
			void SetInitialConditions() override;
#if defined(_EDITOR)
			void WriteJson(nlohmann::json& j) override;
			DECL_CONTROLLER_DRAWER(BrawlerCamera, Controller);
#endif

			void Map(SUUUID so) override;
			void Unmap() override;
			void Step(float delta) override;
			//JS binding
			v8_templates_creators GetV8TemplatesCreators() override;
			v8_context_creators GetV8ContextCreators() override;
			v8_functions_creators GetV8FunctionsCreators() override;

			//object to interact with
			CameraID camera;
			RenderableID venomR;
			float Ycam2venom;
			float YcamInitial;
			bool initialFollowX;
			bool initialFollowY;
		};
	};
};