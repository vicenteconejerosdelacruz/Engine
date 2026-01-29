#pragma once
#include <memory>
#include <string>
#include <Audio.h>
//#include <Application.h>
#include <nlohmann/json.hpp>
//#include <JTypes.h>
#include <Templates.h>
#include <JTemplate.h>
//#include <TemplateDecl.h>

using namespace DirectX;

typedef std::tuple<
	//std::reference_wrapper<std::unique_ptr<DirectX::SoundEffect>>,
	//std::unique_ptr<DirectX::SoundEffect>,
	JUUID,
	std::unique_ptr<DirectX::SoundEffectInstance>
> SoundInstance;

namespace Templates
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <SoundAtt.h>
#include <JEnd.h>

#endif

	namespace Sound
	{
		inline static const std::string templateName = "sounds.json";
		inline static const TemplateType templateType = T_Sounds;
	}

	struct SoundJson : JTemplate
	{
		TEMPLATE_DECL(Sound);

#include <Attributes/JFlags.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <SoundAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
	};

	TEMPDECL_FULL(Sound);

	void SoundJsonStep();
	void ReleaseSoundEffectsInstances();

	SoundInstance GetSoundEffectInstance(
		JUUID uuid,
		unsigned int flags,
		std::string objectUUID = ""
		/*,
		JObjectChangeCallback cb = nullptr,
		JObjectChangePostCallback postCb = nullptr
		*/
	);

	bool SoundEffectExists(JUUID uuid);
	std::unique_ptr<DirectX::SoundEffect>& GetSoundEffect(JUUID uuid);

	void DestroySoundEffectInstance(JUUID uuid, SoundInstance& soundEffectInstance);
};
