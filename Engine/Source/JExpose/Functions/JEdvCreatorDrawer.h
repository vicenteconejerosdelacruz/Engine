
template<typename T, JsonToEditorValueType J>
JEdvCreatorDrawerFunction DrawCreatorValue() { return nullptr; }

template<typename T, JsonToEditorValueType J>
JEdvCreatorDrawerFunction DrawCreatorVector() { return nullptr; }

template<typename Ta, typename Tb>
JEdvCreatorDrawerFunction DrawCreatorMap() { return nullptr; }

template<typename E, JsonToEditorValueType J>
JEdvCreatorDrawerFunction DrawCreatorEnum(
	std::unordered_map<E, std::string>& EtoS,
	std::unordered_map<std::string, E>& StoE
) {
	if (J == jedv_t_hidden) return nullptr;
	return [&EtoS, &StoE](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			auto update = [attribute, &json](auto value)
				{
					nlohmann::json patch = { {attribute,value} };
					json.merge_patch(patch);
				};

			std::string selected = json.at(attribute);
			std::vector<std::string> options;
			std::transform(StoE.begin(), StoE.end(), std::back_inserter(options), [](auto& p) { return p.first; });

			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				ImGui::DrawComboSelection(selected, options, [update](std::string newOption)
					{
						update(newOption);
					}
				);
			}
			ImGui::PopID();
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_string>()
{
	return[](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				ImGui::DrawJsonInputText(json, attribute);
			}
			ImGui::PopID();
		};
}

inline JEdvCreatorDrawerFunction DrawUniqueName(std::string objectName, auto getNames)
{
	return[objectName, getNames](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			auto names = getNames();
			std::set<std::string> namesSet(names.begin(), names.end());
			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				ImGui::DrawJsonInputText(json, attribute);
			}
			ImGui::PopID();
			if (namesSet.contains(json.at(attribute)))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
				std::string text = "*";
				text += objectName;
				text += " with name ";
				text += json.at(attribute);
				text += " Already exists";
				ImGui::Text(text.c_str());
				ImGui::PopStyleColor();
			}
		};
}

inline JEdvCreatorDrawerFunction DrawUniqueSUName(std::string objectName, auto getNames)
{
	return[objectName, getNames](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			auto names = getNames(Editor::currentSceneUnitId);
			std::set<std::string> namesSet(names.begin(), names.end());
			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				ImGui::DrawJsonInputText(json, attribute);
			}
			ImGui::PopID();
			if (namesSet.contains(json.at(attribute)))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
				std::string text = "*";
				text += objectName;
				text += " with name ";
				text += json.at(attribute);
				text += " Already exists";
				ImGui::Text(text.c_str());
				ImGui::PopStyleColor();
			}
		};
}

template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_camera_name>() { return DrawUniqueSUName("Camera", Scene::GetCamerasNames); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_light_name>() { return DrawUniqueSUName("Light", Scene::GetLightsNames); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_renderable_name>() { return DrawUniqueSUName("Renderable", Scene::GetRenderablesNames); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_soundeffect_name>() { return DrawUniqueSUName("SoundEffects", Scene::GetSoundFXsNames); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_physicscene_name>() { return DrawUniqueSUName("PhysicScenes", Scene::GetPhysicScenesNames); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_material_name>() { return DrawCreatorValue<std::string, jedv_t_string>(); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_model3d_name>() { return DrawCreatorValue<std::string, jedv_t_string>(); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_renderpass_name>() { return DrawCreatorValue<std::string, jedv_t_string>(); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_shader_name>() { return DrawCreatorValue<std::string, jedv_t_string>(); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_sound_name>() { return DrawCreatorValue<std::string, jedv_t_string>(); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_texture_name>() { return DrawCreatorValue<std::string, jedv_t_string>(); }

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<bool, jedv_t_boolean>()
{
	return[](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				ImGui::DrawJsonCheckBox(json, attribute);
			}
			ImGui::PopID();
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<unsigned int, jedv_t_sound_instance_flags>()
{
	return [](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			unsigned int currentValue = json.at(attribute);

			for (auto [flag, title] : SOUND_EFFECT_INSTANCE_FLAGSToString)
			{
				bool value = !!(flag & currentValue);
				ImGui::Text(title.c_str());
				ImGui::PushID(title.c_str());
				{
					if (ImGui::Checkbox("##", &value))
					{
						if (value)
							currentValue |= flag;
						else
							currentValue &= ~flag;
					}
				}
				ImGui::PopID();
			}

			json.at(attribute) = currentValue;
		};
}

inline void EditorCreatorDrawTemplateSelector(
	std::string attribute,
	nlohmann::json& json,
	auto GetNameFromUUID,
	auto GetUUIDsNames
)
{
	std::vector<JUUIDName> selectables;
	JUUIDName selected = std::make_tuple("", "");
	selectables.push_back(selected);

	if (json.at(attribute) != "")
	{
		std::string& uuid = std::get<0>(selected);
		std::string& name = std::get<1>(selected);
		uuid = json.at(attribute);
		name = GetNameFromUUID(uuid);
	}
	std::vector<JUUIDName> resources = SortUUIDNameByName(GetUUIDsNames)();
	nostd::AppendToVector(selectables, resources);

	ImGui::Text(attribute.c_str());

	ImGui::PushID(attribute.c_str());
	{
		ImGui::DrawComboSelection(selected, selectables, [attribute, &json](JUUIDName option)
			{
				json.at(attribute) = std::get<0>(option);
			}
		);
	}
	ImGui::PopID();
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_shader>()
{
	return [](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			EditorCreatorDrawTemplateSelector(attribute, json, Templates::GetShaderName, Templates::GetShadersUUIDsNames);
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_sound>()
{
	return [](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			EditorCreatorDrawTemplateSelector(attribute, json, Templates::GetSoundName, Templates::GetSoundsUUIDsNames);
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_material>()
{
	return [](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			EditorCreatorDrawTemplateSelector(attribute, json, Templates::GetMaterialName, Templates::GetMaterialsUUIDsNames);
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_model3d>()
{
	return [](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			EditorCreatorDrawTemplateSelector(attribute, json, Templates::GetModel3DName, Templates::GetModel3DsUUIDsNames);
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_renderpass>()
{
	return [](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			EditorCreatorDrawTemplateSelector(attribute, json, Templates::GetRenderPassName, Templates::GetRenderPasssUUIDsNames);
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_texture>()
{
	return [](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			EditorCreatorDrawTemplateSelector(attribute, json, Templates::GetTextureName, Templates::GetTexturesUUIDsNames);
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorVector<MeshMaterial, jedv_t_vector>()
{
	return [](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			unsigned int size = static_cast<unsigned int>(json.at("meshMaterials").size());
			bool hasModel = json.contains("model") && json.at("model") != "";

			std::vector<JUUIDName> selectablesMeshes = { std::make_tuple("","") };
			std::vector<JUUIDName> selectablesMaterials = { std::make_tuple("","") };
			std::vector<JUUIDName> meshesUUIDs = Templates::GetMeshesUUIDsNames();
			std::vector<JUUIDName> materialsUUIDs = Templates::GetMaterialsUUIDsNames();

			selectablesMeshes.insert(selectablesMeshes.end(), meshesUUIDs.begin(), meshesUUIDs.end());
			selectablesMaterials.insert(selectablesMaterials.end(), materialsUUIDs.begin(), materialsUUIDs.end());

			auto clearMeshMaterial = [&json]
				{
					json.at("meshMaterials").clear();
				};
			auto eraseModel = [&json]
				{
					json.erase("model");
				};
			auto setMesh = [&json, eraseModel](unsigned int index, std::string uuid)
				{
					json.at("meshMaterials").at(index)["mesh"] = uuid;
					if (json.contains("model")) json.erase("model");
					eraseModel();
				};
			auto setMaterial = [&json, eraseModel](unsigned int index, std::string uuid)
				{
					json.at("meshMaterials").at(index)["material"] = uuid;
					if (json.contains("model")) json.erase("model");
					eraseModel();
				};
			auto setModel = [&json, clearMeshMaterial](std::string uuid)
				{
					json["model"] = uuid;
					clearMeshMaterial();
				};
			auto drawRow = [setMesh, setMaterial, meshesUUIDs, materialsUUIDs, selectablesMeshes, selectablesMaterials, &json](unsigned int index)
				{
					std::string mesh = json.at("meshMaterials").at(index).at("mesh");
					std::string material = json.at("meshMaterials").at(index).at("material");
					std::string meshName = mesh.empty() ? "" : Templates::GetMeshName(mesh);
					std::string materialName = material.empty() ? "" : Templates::GetMaterialName(material);

					JUUIDName meshUN = std::make_tuple(mesh, meshName);
					JUUIDName matUN = std::make_tuple(material, materialName);

					ImGui::PushID((std::string("mesh-") + std::to_string(index)).c_str());
					ImGui::DrawComboSelection(meshUN, meshesUUIDs, [index, setMesh](JUUIDName option)
						{
							std::string& nuuid = std::get<0>(option);
							setMesh(index, nuuid);
						}
					);
					ImGui::PopID();

					ImGui::SameLine();
					ImGui::PushID((std::string("material-") + std::to_string(index)).c_str());
					ImGui::DrawComboSelection(matUN, materialsUUIDs, [index, setMaterial](JUUIDName option)
						{
							std::string& nuuid = std::get<0>(option);
							setMaterial(index, nuuid);
						}
					);
					ImGui::PopID();

				};
			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				for (unsigned int i = 0; i < size; i++)
				{
					drawRow(i);
				}

				if (ImGui::Button(ICON_FA_PLUS, ImVec2(ImGui::GetContentRegionAvail().x, 20.0f)))
				{
					json.at("meshMaterials").push_back({
							{ "mesh", "" },
							{ "material", "" },
						}
						);
				}

				ImGui::Text("model");
				std::vector<JUUIDName> selectablesModels = { std::make_tuple("","") };
				std::vector<JUUIDName> model3DsUUIDs = Templates::GetModel3DsUUIDsNames();
				selectablesModels.insert(selectablesModels.end(), model3DsUUIDs.begin(), model3DsUUIDs.end());

				std::string model = json.contains("model") ? json.at("model") : "";
				std::string modelName = model.empty() ? "" : Templates::GetModel3DName(model);

				JUUIDName modelUN = std::make_tuple(model, modelName);

				ImGui::PushID(std::string("model").c_str());
				ImGui::DrawComboSelection(modelUN, model3DsUUIDs, [setModel](JUUIDName option)
					{
						std::string& nuuid = std::get<0>(option);
						setModel(nuuid);
					}
				);
				ImGui::PopID();

			}
			ImGui::PopID();
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorVector<std::string, jedv_t_so_camera_vector>()
{
	return [](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			unsigned int size = static_cast<unsigned int>(json.at(attribute).size());
			std::set<std::string> currentUUIDs;
			for (unsigned int i = 0; i < size; i++)
			{
				currentUUIDs.insert(json.at(attribute).at(i));
			}
			//std::vector<JUUIDName> camsUUIDNames = GetSceneObjectsByType(Editor::currentSceneUnitId, SO_Cameras)();
			std::vector<JUUIDName> camsUUIDNames = GetSUSceneObjectsByType(Editor::currentSceneUnitId, SO_Cameras);

			auto setCamera = [&json, attribute](unsigned int index, std::string uuid)
				{
					json.at(attribute)[index] = uuid;
				};

			auto drawRow = [&json, attribute, &camsUUIDNames, &currentUUIDs, setCamera](unsigned int index)
				{
					std::string uuid = json.at(attribute).at(index);
					std::vector<JUUIDName> selectablesCams = { std::make_tuple("","") };
					std::copy_if(camsUUIDNames.begin(), camsUUIDNames.end(), std::back_inserter(selectablesCams), [uuid, &currentUUIDs](JUUIDName cam)
						{
							std::string camUUID = std::get<0>(cam);
							return !currentUUIDs.contains(camUUID) || uuid == camUUID;
						}
					);
					JUUIDName selected = std::make_tuple("", "");
					if (uuid != "")
					{
						for (unsigned int i = 0; i < selectablesCams.size(); i++)
						{
							if (std::get<0>(selectablesCams[i]) == uuid)
							{
								selected = selectablesCams[i];
							}
						}
					}
					ImGui::PushID((std::string("camera-") + std::to_string(index)).c_str());
					ImGui::DrawComboSelection(selected, selectablesCams, [index, setCamera](JUUIDName option)
						{
							std::string& nuuid = std::get<0>(option);
							setCamera(index, nuuid);
						}
					);
					ImGui::PopID();
				};

			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());

				for (unsigned int i = 0; i < size; i++)
				{
					drawRow(i);
				}
			}
			ImGui::PopID();
			if (json.at(attribute).size() < camsUUIDNames.size())
			{
				if (ImGui::Button(ICON_FA_PLUS, ImVec2(ImGui::GetContentRegionAvail().x, 20.0f)))
				{
					json.at(attribute).push_back("");
				}
			}
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorVector<DXGI_FORMAT, jedv_t_dxgi_format_vector>()
{
	return [](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			unsigned int size = static_cast<unsigned int>(json.at(attribute).size());

			std::vector<std::string> selectables = { "" };
			std::vector<std::string> formats = nostd::GetKeysFromMap(StringToDXGI_FORMAT);
			nostd::AppendToVector(selectables, formats);

			auto drawRow = [&json, attribute, &selectables](unsigned int row)
				{
					std::string selected = json.at(attribute).at(row);
					ImGui::PushID(std::string(std::string("format-") + std::to_string(row)).c_str());
					ImGui::DrawComboSelection(selected, selectables, [&json, attribute, row](std::string option)
						{
							json.at(attribute).at(row) = option;
						}, "##");
					ImGui::PopID();
				};

			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());

				for (unsigned int i = 0; i < size; i++)
				{
					drawRow(i);
				}
			}
			ImGui::PopID();

			if (json.at(attribute).size() < 8) //(MAX RENDER PASSES?)
			{
				if (ImGui::Button(ICON_FA_PLUS, ImVec2(ImGui::GetContentRegionAvail().x, 20.0f)))
				{
					json.at(attribute).push_back("");
				}
			}
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorEnum<LightType, jedv_t_lighttype>(
	std::unordered_map<LightType, std::string>& EtoS,
	std::unordered_map<std::string, LightType>& StoE
) {
	return [&EtoS, &StoE](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			DrawCreatorValue<std::string, jedv_t_so_light_name>()("name", json, modalProperties);

			auto update = [attribute, &json, &StoE](auto value)
				{
					nlohmann::json patch = { {attribute,value} };
					json.merge_patch(patch);
					if (StoE.at(json.at(attribute)) == LT_Ambient)
					{
						nlohmann::json patchSM = { {"hasShadowMaps",false} };
						json.merge_patch(patchSM);
					}
				};
			std::string selected = json.at(attribute);
			std::vector<std::string> options;
			std::transform(StoE.begin(), StoE.end(), std::back_inserter(options), [](auto& p) { return p.first; });

			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				ImGui::DrawComboSelection(selected, options, [update](std::string newOption)
					{
						update(newOption);
					}
				);
			}
			ImGui::PopID();

			float color[3];
			for (unsigned int i = 0; i < 3; i++)
			{
				color[i] = json.at("color").at(i);
			}
			ImGui::PushID("color");
			{
				ImGui::Text("color");
				if (ImGui::ColorEdit3("##", color))
				{
					nlohmann::json patch = { {"color",nlohmann::json::array({color[0],color[1],color[2]})} };
					json.merge_patch(patch);
				}
			}
			ImGui::PopID();

			if (StoE.at(selected) != LT_Ambient)
			{
				ImGui::PushID("hasShadowMaps");
				{
					ImGui::Text("hasShadowMaps");
					bool value = json.at("hasShadowMaps");
					if (ImGui::Checkbox("##", &value))
					{
						nlohmann::json patchSM = { {"hasShadowMaps",value} };
						json.merge_patch(patchSM);
					}
				}
				ImGui::PopID();
			}
		};
}

inline bool EditorCreatorDrawFilePath(
	std::string attribute,
	nlohmann::json& json,
	std::string attText,
	const char* buttonIcon,
	nlohmann::json& modalProperties,
	std::vector<std::string> filterName,
	std::vector<std::string> filterPattern
)
{
	bool ret = false;
	auto getFilePath = [attribute, &json]()
		{
			return json.at(attribute);
		};
	auto setFilePath = [&ret, attribute, &json](std::string path)
		{
			nlohmann::json patch = { {attribute,path} };
			json.merge_patch(patch);
			ret = true;
		};

	std::string defaultFolder = modalProperties.at("assetsFolder");
	std::string fileFolder = modalProperties.at("fileFolder");

	ImGui::PushID(attribute.c_str());
	{
		ImGui::Text(attText.c_str());
		std::filesystem::path path = getFilePath();

		if (ImGui::Button(buttonIcon))
		{
			ImGui::OpenFile([setFilePath, defaultFolder, &modalProperties](std::filesystem::path p)
				{
					std::filesystem::path absfilepath = std::filesystem::current_path().append(defaultFolder);
					std::filesystem::path rel = std::filesystem::relative(p, absfilepath);
					setFilePath(rel.generic_string());
					modalProperties.at("fileFolder") = rel.parent_path().generic_string();
				}, fileFolder, filterName, filterPattern);
		}
		ImGui::SameLine();
		ImGui::InputText("##", path.string().data(), path.string().size(), ImGuiInputTextFlags_ReadOnly);
	}
	ImGui::PopID();
	return ret;
}

inline bool EditorCreatorDrawFilePath(
	std::string attribute,
	nlohmann::json& json,
	unsigned int attIndex,
	std::string attText,
	const char* buttonIcon,
	nlohmann::json& modalProperties,
	std::vector<std::string> filterName,
	std::vector<std::string> filterPattern
)
{
	bool ret = false;
	auto getFilePath = [attribute, &json, attIndex]()
		{
			return json.at(attribute).at(attIndex);
		};
	auto setFilePath = [&ret, attribute, &json, attIndex](std::string path)
		{
			json.at(attribute)[attIndex] = path;
			ret = true;
		};

	std::string defaultFolder = modalProperties.at("assetsFolder");
	std::string fileFolder = modalProperties.at("fileFolder");

	ImGui::PushID(attribute.c_str());
	{
		ImGui::Text(attText.c_str());
		std::filesystem::path path = getFilePath();

		if (ImGui::Button(buttonIcon))
		{
			ImGui::OpenFile([setFilePath, defaultFolder, &modalProperties](std::filesystem::path p)
				{
					p = std::filesystem::canonical(p);
					std::filesystem::path currpath = std::filesystem::canonical(std::filesystem::current_path());
					std::filesystem::path absfilepath = std::filesystem::canonical(currpath.append(defaultFolder));
					std::filesystem::path rel = std::filesystem::relative(p, absfilepath);
					setFilePath(rel.generic_string());
					modalProperties.at("fileFolder") = rel.parent_path().generic_string();
				}, fileFolder, filterName, filterPattern);
		}
		ImGui::SameLine();
		ImGui::InputText("##", path.string().data(), path.string().size(), ImGuiInputTextFlags_ReadOnly);
	}
	ImGui::PopID();
	return ret;
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_shaders_filepath>()
{
	return[](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			EditorCreatorDrawFilePath(attribute, json, attribute, ICON_FA_FILE_CODE, modalProperties, { "HLSL files. (*.hlsl)" }, { "*.hlsl" });
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_sounds_filepath>()
{
	return[](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			EditorCreatorDrawFilePath(attribute, json, attribute, ICON_FA_FILE_AUDIO, modalProperties,
				{ "WAV files. (*.wav)", "MP3 files. (*.mp3)", "OGG files. (*.ogg)" },
				{ "*.wav", "*.mp3", "*.ogg" });
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_model3d_filepath>()
{
	return[](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			EditorCreatorDrawFilePath(attribute, json, attribute, ICON_FA_CUBE, modalProperties,
				{ "Gltf files. (*.gltf)", "Glb files. (*.glb)" },
				{ "*.gltf", "*.glb" }
			);
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorVector<std::string, jedv_t_filepath_vector_image>()
{
	return [](std::string attribute, nlohmann::json& json, nlohmann::json& modalProperties)
		{
			TextureType oldType = StringToTextureType.at(json.at("type"));

			ImGui::Text("type");
			ImGui::PushID("type");
			ImGui::DrawComboSelection(json, "type", nostd::GetKeysFromMap(StringToTextureType));
			ImGui::PopID();

			TextureType newType = StringToTextureType.at(json.at("type"));

			if (!json.contains("createIrradiance")) json["createIrradiance"] = false;
			if (!json.contains("createPrefilteredEnv")) json["createPrefilteredEnv"] = false;
			if (!json.contains("createBRDFLut")) json["createBRDFLut"] = false;

			if (newType != oldType || json.at("images").size() == 0ULL)
			{
				json.at("name") = "";
				json["createIrradiance"] = false;
				json["createPrefilteredEnv"] = false;
				json["createBRDFLut"] = false;
				switch (newType)
				{
				case TextureType_Cube:
				{
					json.at("images") = { "","","","","","" };
				}
				break;
				default:
				{
					json.at("images") = { "" };
				}
				break;
				}
			}

			ImGui::Text("name");
			std::string name = json.at("name");
			switch (newType)
			{
			case TextureType_2D:
			case TextureType_Skybox:
			case TextureType_Array:
			{
				ImGui::PushID(std::string(attribute + "-input-text").c_str());
				ImGui::InputText("##", name.data(), name.size(), ImGuiInputTextFlags_ReadOnly);
				ImGui::PopID();
				ImGui::PushID(std::string(attribute + "-pick-file-0").c_str());
				if (EditorCreatorDrawFilePath(attribute, json, 0, attribute, ICON_FA_FILE_IMAGE, modalProperties,
					(newType != TextureType_Array) ? defaultTexturesFilters : defaultAnimatedTexturesFilters,
					(newType != TextureType_Array) ? defaultTexturesExtensions : defaultAnimatedTexturesExtensions
				))
				{
					json.at("name") = json.at(attribute).at(0);
				}
				ImGui::PopID();
			}
			break;
			case TextureType_Cube:
			{
				ImGui::PushID(std::string(attribute + "-input-text").c_str());
				ImGui::InputText("##", json.at("name").get_ptr<std::string*>());
				ImGui::PopID();

				for (unsigned int i = 0; i < 6U; i++)
				{
					ImGui::PushID(std::string(attribute + "-pick-file-" + std::to_string(i)).c_str());
					if (EditorCreatorDrawFilePath(attribute, json, i, cubeTextureAxesNames.at(i), ICON_FA_FILE_IMAGE, modalProperties, defaultTexturesFilters, defaultTexturesExtensions))
					{
						if (i == 0)
						{
							json.at("name") = json.at(attribute).at(0);
						}
					}
					ImGui::PopID();
				}
			}
			break;
			}

			switch (newType)
			{
			case TextureType_Skybox:
			case TextureType_Cube:

				ImGui::PushID("createIrradiance");
				{
					ImGui::Text("irradiance");
					ImGui::DrawJsonCheckBox(json, "createIrradiance");
				}
				ImGui::PopID();

				ImGui::PushID("createPrefilteredEnv");
				{
					ImGui::Text("prefiltered env");
					ImGui::DrawJsonCheckBox(json, "createPrefilteredEnv");
				}
				ImGui::PopID();

				ImGui::PushID("createBRDFLut");
				{
					ImGui::Text("BRDF LUT");
					ImGui::DrawJsonCheckBox(json, "createBRDFLut");
				}
				ImGui::PopID();

				break;
			}
		};
}