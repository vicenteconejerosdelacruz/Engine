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
#include "BrawlerCameraAtt.h"
#include <JEnd.h>
#endif

	static BrawlerScene* GetBrawlerScene(BrawlerCamera* cam)
	{
		return GetController<BrawlerScene>(cam->unit, cam->sceneController());
	}

	BrawlerCamera::BrawlerCamera(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include "BrawlerCameraAtt.h"
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include "BrawlerCameraAtt.h"
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include "BrawlerCameraAtt.h"
#include <JEnd.h>
	}

	void BrawlerCamera::RegisterScript(Isolate* isolate, Local<ObjectTemplate> tpl, SceneUnitScripting* script)
	{
		v8_register_method<BrawlerCamera>(isolate, tpl, "TweenSmoothing", script, [](BrawlerCamera* self, float from, float to, float time) { if (self) self->TweenSmoothing(from, to, time); });
	}

	void BrawlerCamera::SetInitialConditions()
	{
		if (fromPlayMode)
		{
			followLeft(initialFollowLeft);
			followRight(initialFollowRight);
			followUp(initialFollowUp);
			followDown(initialFollowDown);
		}
		leftBoundaryB = MAKESUUUID(unit, leftBoundary());
		rightBoundaryB = MAKESUUUID(unit, rightBoundary());
		topBoundaryB = MAKESUUUID(unit, topBoundary());
		leftBoundaryPO = leftBoundaryB->physicObject;
		rightBoundaryPO = rightBoundaryB->physicObject;
		topBoundaryPO = topBoundaryB->physicObject;
	}

#if defined(_EDITOR)
	void BrawlerCamera::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include "BrawlerCameraAtt.h"
#include <JEnd.h>
		Controller::WriteJson(j);
	}
	void BrawlerCamera::SwitchToPlayMode()
	{
		fromPlayMode = true;
		initialFollowLeft = followLeft();
		initialFollowRight = followRight();
		initialFollowUp = followUp();
		initialFollowDown = followDown();
	}
#endif

	void BrawlerCamera::Map(SUUUID so)
	{
		Controller::Map(so);

		camera = so;
		auto* scene = GetBrawlerScene(this);
		RenderableID heroR = GetController<Hero>(*scene->heroes().begin())->sceneObject;
		cameraOffset = XMVectorSubtract(camera->positionV(), heroR->positionV());

		SetInitialConditions();
	}

	void BrawlerCamera::Unmap()
	{
		Controller::Unmap();
		camera.clear();
	}

	void BrawlerCamera::Step(float delta)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
			return;
#endif
		BrawlerScene* scene = GetBrawlerScene(this);
		const std::set<JUUID>& heroIDs = scene->heroes();

		if (smoothingTween != nullptr)
		{
			smoothingTween->step();
			smoothing(smoothingTween->current_value);
			if (smoothingTween->current_value == smoothingTween->target_value)
			{
				smoothingTween = nullptr;
			}
		}

		if (heroIDs.empty())
			return;

		// 1. Calcular el centro de la caja que contiene a todos los héroes
		float minX = FLT_MAX, maxX = -FLT_MAX;
		float minY = FLT_MAX, maxY = -FLT_MAX;
		float minZ = FLT_MAX, maxZ = -FLT_MAX;

		for (const auto& id : heroIDs)
		{
			Hero* hero = GetController<Hero>(id);
			if (!hero)
				continue;

			RenderableID heroR = hero->sceneObject;
			XMVECTOR pos = heroR->positionV();

			float x = XMVectorGetX(pos);
			float y = XMVectorGetY(pos);
			float z = XMVectorGetZ(pos);

			if (x < minX) minX = x; if (x > maxX) maxX = x;
			if (y < minY) minY = y; if (y > maxY) maxY = y;
			if (z < minZ) minZ = z; if (z > maxZ) maxZ = z;
		}

		// Centro del grupo (El punto en el mundo que queremos que esté en el centro de la pantalla)
		float targetX = (minX + maxX) * 0.5f;
		float targetY = (minY + maxY) * 0.5f;
		float targetZ = (minZ + maxZ) * 0.5f;

		// 2. Posición actual de la cámara
		XMVECTOR camPosV = camera->positionV();
		float curCamX = XMVectorGetX(camPosV);
		float curCamY = XMVectorGetY(camPosV);
		float curCamZ = XMVectorGetZ(camPosV);

		// 3. Obtener el desplazamiento fijo que define tu vista
		// Este vector determina la distancia y el ángulo relativo al objetivo.
		float offsetX = XMVectorGetX(cameraOffset);
		float offsetY = XMVectorGetY(cameraOffset);
		float offsetZ = XMVectorGetZ(cameraOffset);

		// 4. CALCULAR POSICIÓN DESEADA DE LA CÁMARA
		// Sumamos el offset al centro de los héroes
		float desiredCamX = targetX + offsetX;
		float desiredCamY = targetY + offsetY;
		//float desiredCamZ = targetZ + offsetZ;
		float desiredCamZ = curCamZ;

		// 5. FILTRADO CON LAS FUNCIONES DE CONTROL
		// Comparamos hacia dónde quiere ir la cámara respecto a dónde está ahora

		// Eje X: Desplazamiento lateral
		if (desiredCamX < curCamX && !followLeft())  desiredCamX = curCamX;
		if (desiredCamX > curCamX && !followRight()) desiredCamX = curCamX;

		// Eje Y: Desplazamiento vertical (Salto o elevación)
		if (desiredCamY > curCamY && !followUp())    desiredCamY = curCamY;
		if (desiredCamY < curCamY && !followDown())  desiredCamY = curCamY;

		// Eje Z: Desplazamiento en profundidad
		// (Aplica tus flags aquí si el movimiento Far/Near también debe bloquearse)

		// 6. Aplicar movimiento suave (Lerp)
		float lerpFactor = std::clamp(smoothing() * delta, 0.0f, 1.0f);
		XMVECTOR nextPos = XMVectorSet(desiredCamX, desiredCamY, desiredCamZ, 0.0f);

		XMVECTOR lerpedPosV = XMVectorLerp(camPosV, nextPos, lerpFactor);
		XMFLOAT3 lerpedPos;
		XMStoreFloat3(&lerpedPos, lerpedPosV);

		XMVECTOR diff = XMVectorSubtract(lerpedPosV, camPosV);

		if (XMVectorGetX(XMVector3Length(diff)) > 0.0001f)
		{
			MoveCameraBoundary(diff, leftBoundaryB);
			MoveCameraBoundary(diff, rightBoundaryB);
			MoveCameraBoundary(diff, topBoundaryB);
		}

		camera->position(lerpedPos);
	}

	void BrawlerCamera::MoveCameraBoundary(XMVECTOR diff, BoundaryID boundary)
	{
		XMVECTOR newPos = XMVectorAdd(boundary->positionV(), diff);
		XMFLOAT3 xmnewPos;
		XMStoreFloat3(&xmnewPos, newPos);
		boundary->position(xmnewPos);
		boundary->flag(Boundary::Update_position);
	}

	void BrawlerCamera::TweenSmoothing(float from, float to, float time)
	{
		smoothingTween = std::make_unique<tween>(from, to, static_cast<int>(1000.0f * time), tween::easing::linear);
	}
}

