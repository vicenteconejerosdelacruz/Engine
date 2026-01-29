#include "pch.h"
#include "Sound.h"
#include <map>
#include <set>
#include <Audio.h>
#include <AudioSystem.h>
//#include <Templates.h>
//#include <TemplateDef.h>

using namespace AudioSystem;
using namespace DirectX;

namespace Templates
{
#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <SoundAtt.h>
#include <JEnd.h>

#endif

	namespace Sound
	{
		std::unordered_map<JUUID, std::unique_ptr<DirectX::SoundEffect>> uuidToSoundEffects;
		std::unordered_map<JUUID, unsigned int> uuidInstanceCount;
	};

	SoundInstance GetSoundEffectInstance(JUUID uuid, unsigned int flags,
		std::string objectUUID/*, JObjectChangeCallback cb, JObjectChangePostCallback postCb*/
	)
	{
		/*if (objectUUID != "" && (cb != nullptr || postCb != nullptr))
		{
			std::unique_ptr<SoundJson>& json = GetSoundTemplate(uuid);
			json->BindChangeCallback(objectUUID, cb, postCb);
		}*/
		using namespace Sound;
		if (uuidToSoundEffects.contains(uuid))
		{
			uuidInstanceCount[uuid]++;
		}
		else
		{
			std::unique_ptr<SoundJson>& json = GetSoundTemplate(uuid);
			std::wstring path = nostd::StringToWString(defaultSoundsFolder + json->path());
			uuidToSoundEffects[uuid] = std::make_unique<DirectX::SoundEffect>(GetAudioEngine().get(), path.c_str());
			uuidInstanceCount[uuid] = 1;
		}
		//std::unique_ptr<SoundJson>& json = GetSoundTemplate(uuid);
		//OutputDebugStringA(std::string(std::string("create") + json->name() + ":" + std::to_string(uuidInstanceCount[uuid]) + "\n").c_str());
		return std::make_tuple(
			uuid,
			std::move(uuidToSoundEffects[uuid]->CreateInstance(SOUND_EFFECT_INSTANCE_FLAGS(flags)))
		);
	}

	bool SoundEffectExists(JUUID uuid)
	{
		using namespace Sound;
		return uuidToSoundEffects.contains(uuid);
	}

	std::unique_ptr<DirectX::SoundEffect>& GetSoundEffect(JUUID uuid)
	{
		using namespace Sound;
		return uuidToSoundEffects.at(uuid);
	}

	void DestroySoundEffectInstance(JUUID uuid, SoundInstance& soundEffectInstance)
	{
		using namespace Sound;
		uuidInstanceCount[uuid]--;
		//std::unique_ptr<SoundJson>& json = GetSoundTemplate(uuid);
		//OutputDebugStringA(std::string(std::string("destroy") + json->name() + ":" + std::to_string(uuidInstanceCount[uuid]) + "\n").c_str());
		//std::unique_ptr<DirectX::SoundEffectInstance>& sfxI = std::get<1>(soundEffectInstance);
		//sfxI = nullptr;
		soundEffectInstance = std::make_tuple("", nullptr);
		if (uuidInstanceCount[uuid] == 0)
		{
			uuidToSoundEffects.erase(uuid);
			uuidInstanceCount.erase(uuid);
		}
	}

	SoundJson::SoundJson(nlohmann::json& json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <SoundAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void SoundJson::WriteJson(nlohmann::json& j)
	{
#include <Editor/JWriteJson.h>
#include <SoundAtt.h>
#include <JEnd.h>
	}
#endif

	TEMPDEF_FULL(Sound);

	void SoundJsonStep()
	{
		std::set<SoundJsonUUID> sounds;
		std::transform(Soundtemplates.begin(), Soundtemplates.end(), std::inserter(sounds, sounds.begin()), [](auto& temps)
			{
				return temps.first;
			}
		);

		std::set<SoundJsonUUID> rebuildSounds;
		std::copy_if(sounds.begin(), sounds.end(), std::inserter(rebuildSounds, rebuildSounds.begin()), [](auto sound)
			{
				return sound->dirty(SoundJson::Update_path);
			}
		);

		if (rebuildSounds.size() > 0ULL)
		{
			/*
			JObject::RunChangesCallback(rebuildSounds, [](auto sound)
				{
					sound->clean(SoundJson::Update_path);
				}
			);
			*/
		}
	}

	void ReleaseSoundEffectsInstances()
	{
		//for (auto& [uuid, t] : sounds)
		//{
		//	auto& instances = std::get<3>(t);
		//	instances.clear();
		//}
	}
}
