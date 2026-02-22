#ifndef _TEMPLATES_SOUND_H
#define _TEMPLATES_SOUND_H

#include <memory>
#include <string>
#include <Audio.h>
#include <nlohmann/json.hpp>
#include <Templates.h>
#include <JTemplate.h>

using namespace DirectX;

typedef std::tuple<JUUID, std::unique_ptr<DirectX::SoundEffectInstance>> SoundInstance;

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
	DEF_TEMPLATE_ID(SoundJson, GetSoundTemplate);

	void SoundJsonStep();
	void ReleaseSoundEffectsInstances();

	SoundInstance GetSoundEffectInstance(
		JUUID uuid,
		unsigned int flags,
		std::string objectUUID = ""
	);

	bool SoundEffectExists(JUUID uuid);
	std::unique_ptr<DirectX::SoundEffect>& GetSoundEffect(JUUID uuid);

	void DestroySoundEffectInstance(JUUID uuid, SoundInstance& soundEffectInstance);
};
using namespace Templates;
DEF_TEMPLATE_ID_HASH(SoundJson);

#endif