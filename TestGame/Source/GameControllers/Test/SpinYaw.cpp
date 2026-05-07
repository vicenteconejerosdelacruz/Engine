#include "pch.h"
#include "SpinYaw.h"
#include <GamePad.h>
#include <Scene.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

extern std::unique_ptr<DirectX::GamePad> gamePad;
extern DirectX::GamePad::ButtonStateTracker buttons;

namespace Game::Test
{
#if defined(_EDITOR)
#include <Editor/JDrawersDef.h>
#include <Test/SpinYawAtt.h>
#include <JEnd.h>
#endif

	SpinYaw::SpinYaw(nlohmann::json& json) : Controller(json)
	{
#include <Attributes/JInit.h>
#include <Test/SpinYawAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <Test/SpinYawAtt.h>
#include <JEnd.h>

#include <Attributes/JV8Att.h>
#include <Test/SpinYawAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void SpinYaw::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <Test/SpinYawAtt.h>
#include <JEnd.h>
		Controller::WriteJson(j);
	}
#endif

	void SpinYaw::Step(float delta)
	{
		using namespace Scene;

		//auto o = GetSceneObjectPointer(sceneObject);
		SceneUnitId unit = std::get<0>(sceneObject);
		RenderableID o = sceneObject;

		if (!o->contains("rotation"))
			return;

		auto state = gamePad->GetState(0);
		if (!state.IsConnected())
		{
			buttons.Reset();
			return;
		}

		buttons.Update(state);
#if defined(_EDITOR)
		if (!Editor::IsPlaying(unit) || Editor::IsPaused(unit))
			return;
#endif
		auto pad = gamePad->GetState(0);
		float dy = pad.thumbSticks.leftX * speed();

		XMFLOAT3 rot = ToXMFLOAT3(o->at("rotation"));
		rot.y += dy;
		o->at("rotation") = FromXMFLOAT3(rot);
	}
}

