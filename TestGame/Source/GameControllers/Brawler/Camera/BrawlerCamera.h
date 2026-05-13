#pragma once
#include <Controller.h>
//#include "../Rounds/BrawlerRound.h"

namespace Game
{
	namespace Brawler
	{
#if defined(_EDITOR)
#include <Attributes/JOrder.h>
#include "BrawlerCameraAtt.h"
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include "BrawlerCameraAtt.h"
#include <JEnd.h>
#endif
		struct Venom;
		struct BrawlerCamera : Controller
		{
#include <Attributes/JFlags.h>
#include "BrawlerCameraAtt.h"
#include <JEnd.h>

#include <Attributes/JStr2Flag.h>
#include "BrawlerCameraAtt.h"
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include "BrawlerCameraAtt.h"
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
			void MoveCameraBoundary(XMVECTOR diff, BoundaryID boundary);

			//object to interact with
			CameraID camera;
			XMVECTOR cameraOffset;
			//RenderableID heroes[2];
			//Venom* venomC;
			//float Ycam2venom;
			//float YcamInitial;
			BoundaryID leftBoundaryB;
			BoundaryID rightBoundaryB;
			BoundaryID topBoundaryB;
			PhysicObjectID leftBoundaryPO;
			PhysicObjectID rightBoundaryPO;
			PhysicObjectID topBoundaryPO;
		};
	};
};