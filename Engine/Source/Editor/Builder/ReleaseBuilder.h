#pragma once
#include <JTypes.h>
#include <Templates.h>
#include <UUID.h>

template<JsonToEditorValueType T, typename Extra>
struct ReleaseBuilder
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream) {}
};
template<>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_mesh_material, MeshMaterial>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		if (!att.contains("material") || att.at("material").empty())
			return;

		JUUID t = att.at("material");
		if (!TemplateExists(t) || templates.contains(t))
			return;

		MaterialJsonID m = t;
		logStream << "material:" + m->name() + "\n";
		m->GatherFiles(filesToCopy, templates, logStream);
	}
};
template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_te_model3d, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		if (att.empty())
			return;

		JUUID t = std::string(att);
		if (!TemplateExists(t) || templates.contains(t))
			return;

		templates.insert(t);

		Model3DJsonID model = t;
		logStream << "model:" + model->name() + "\n";
		model->GatherFiles(filesToCopy, templates, logStream);
	}
};
template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_model3d_filepath, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		std::filesystem::path path = default3DModelsFolder + std::string(att);
		if (filesToCopy.contains(path))
			return;

		filesToCopy.insert(path);
		logStream << "file:" + path.string() + "\n";
		if (path.extension() == ".gltf")
		{
			std::ifstream file(path);
			nlohmann::json gltfContent = nlohmann::json::parse(file);
			if (gltfContent.contains("buffers"))
			{
				std::string buff_file = gltfContent.at("buffers").at(0).at("uri");
				std::filesystem::path buff_path = path.parent_path().string() + "/" + buff_file;
				filesToCopy.insert(buff_path);
				logStream << "file:" + buff_path.string() + "\n";
			}
		}
	}
};
template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_sounds_filepath, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		std::filesystem::path path = defaultSoundsFolder + std::string(att);
		if (filesToCopy.contains(path))
			return;

		filesToCopy.insert(path);
		logStream << "file:" + path.string() + "\n";
	}
};
template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_te_shader, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		templates.insert(std::string(att));
	}
};
template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_te_material_vector, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		for (unsigned int idx = 0; idx < att.size(); idx++)
		{
			JUUID m = JUUID(att.at(idx));
			if (templates.contains(m))
				continue;

			templates.insert(m);

			MaterialJsonID mat = m;
			logStream << "material:" + mat->name() + "\n";
			mat->GatherFiles(filesToCopy, templates, logStream);
		}
	}
};
template<>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_map, TextureShaderUsage>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		for (auto& [use, _] : StringToTextureShaderUsage)
		{
			if (!att.contains(use))
				continue;

			JUUID t = JUUID(att.at(use));
			if (!TemplateExists(t) || templates.contains(t))
				continue;

			templates.insert(t);

			TextureJsonID tex = t;
			logStream << "texture:" + tex->name() + "\n";
			tex->GatherFiles(filesToCopy, templates, logStream);
		}
	}
};
template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_te_texture, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		if (att.empty())
			return;

		JUUID t = att;
		if (!TemplateExists(t) || templates.contains(t))
			return;

		templates.insert(t);

		TextureJsonID tex = t;
		logStream << "texture:" + tex->name() + "\n";
		tex->GatherFiles(filesToCopy, templates, logStream);
	}
};
template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_filepath_vector_image, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		for (unsigned int idx = 0; idx < att.size(); idx++)
		{
			std::string path = att.at(idx);
			if (path.empty())
				continue;

			std::filesystem::path fpath = path;
			fpath.replace_extension(".dds");

			filesToCopy.insert(fpath);
			logStream << "image:" + fpath.string() + "\n";
		}
	}
};

template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_te_sound, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		if (att.empty())
			return;

		JUUID t = std::string(att);
		if (!TemplateExists(t) || templates.contains(t))
			return;

		templates.insert(t);

		SoundJsonID sound = t;
		logStream << "sound:" + sound->name() + "\n";
		sound->GatherFiles(filesToCopy, templates, logStream);
	}
};

template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_te_mold, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		if (att.empty())
			return;

		JUUID t = std::string(att);
		if (!TemplateExists(t) || templates.contains(t))
			return;

		templates.insert(t);

		MoldJsonID mold = t;
		logStream << "mold:" + mold->name() + "\n";
		mold->GatherFiles(filesToCopy, templates, logStream);
	}
};

template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_te_htmlui, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		if (att.empty())
			return;

		JUUID t = std::string(att);
		if (!TemplateExists(t) || templates.contains(t))
			return;

		templates.insert(t);

		HtmlUIJsonID html = t;
		logStream << "html:" + html->name() + "\n";
		html->GatherFiles(filesToCopy, templates, logStream);
	}
};

template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_htmls_filepath, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		if (att.empty())
			return;

		std::filesystem::path path = defaultUIFolder + std::string(att);
		if (filesToCopy.contains(path))
			return;

		// Obtenemos el directorio padre del archivo path
		std::filesystem::path directory = path.parent_path();

		// Verificamos que el directorio exista en el disco
		if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory))
			return;

		// Iteramos de manera recursiva por todo el contenido del directorio
		for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			// Nos aseguramos de procesar solo archivos regulares (ignorando subcarpetas vacías)
			if (!entry.is_regular_file())
				continue;

			std::filesystem::path filePath = entry.path();
			if (filesToCopy.contains(filePath))
				continue;

			// Intentamos insertar el archivo en el set compartido
			filesToCopy.insert(filePath);
			logStream << "html resource:" << filePath.string() << "\n";
		}
	}
};

template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_physic_object_vector, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		if (att.empty())
			return;

		if (att.is_array())
		{
			for (unsigned int idx = 0U; idx < att.size(); idx++)
			{
				PhysicObject::GatherFiles(att.at(idx), filesToCopy, templates, logStream);
			}
		}
	}
};

template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_te_physycgeometry, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		if (att.empty())
			return;

		JUUID t = std::string(att);
		if (!TemplateExists(t) || templates.contains(t))
			return;

		templates.insert(t);

		PhysicGeometryJsonID phg = t;
		logStream << "physicGeometry:" + phg->name() + "\n";
		phg->GatherFiles(filesToCopy, templates, logStream);
	}
};

template<>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_hidden, AnimationSequences>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		if (att.empty())
			return;

		for (auto& [_, seq] : att.items())
		{
			if (!seq.contains("sequenceChannels") || seq.at("sequenceChannels").size() == 0)
				continue;

			for (int ch = 0; ch < seq.at("sequenceChannels").size(); ch++)
			{
				auto& channel = seq.at("sequenceChannels").at(ch);
				auto& elems = channel.at("elements");
				for (int i = 0; i < elems.size(); i++)
				{
					if (!elems.at(i).contains("soundfx"))
						continue;

					JUUID t = elems.at(i).at("soundfx").at("sound");
					if (!TemplateExists(t) || templates.contains(t))
						continue;

					SoundJsonID sound = t;
					sound->GatherFiles(filesToCopy, templates, logStream);
				}
			}
		}
	}
};

extern std::unordered_map<std::string, std::function<void(nlohmann::json& controller, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)>> Game::controllerReleaseBuilders;
template<typename Extra>
struct ReleaseBuilder<JsonToEditorValueType::jedv_t_controller_vector, Extra>
{
	static void GatherFiles(nlohmann::json& att, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		for (auto& [name, controller] : att.items())
		{
			if (!controllerReleaseBuilders.contains(name))
				continue;

			controllerReleaseBuilders.at(name)(controller, filesToCopy, templates, logStream);
		}
	}
};