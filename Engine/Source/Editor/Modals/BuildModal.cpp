#include "pch.h"
#include <imgui.h>
#include "BuildModal.h"
#include <ImEditor.h>
#include <filesystem>
#include <Application.h>
#include <Level.h>
#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <Renderable/Renderable.h>

namespace fs = std::filesystem;

void BuildModal::Init()
{
	SimpleModal::Init(ImVec2(0.4f, 0.6f), "Build");
	files = GetYamlFiles(defaultLevelsFolder);
	includeFlags.resize(files.size());
	bootLevelSelected = -1;
	building = false;
	completed = false;
}

void BuildModal::Draw()
{
	if (!showing) return;

	bool canClick = (!building || completed) && bootLevelSelected != -1 && std::any_of(includeFlags.begin(), includeFlags.end(), [](bool v) {return v; }) && includeFlags.at(bootLevelSelected);

	SimpleModal::Draw([&](ImVec2 size)
		{
			if (!building)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
				ImGui::BeginChild("script-binding", size, 0);
				{
					if (ImGui::BeginTable("BuildFilesTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
					{
						// Definir los headers de las columnas
						ImGui::TableSetupColumn("Include", ImGuiTableColumnFlags_WidthFixed, 60.0f);
						ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthStretch);
						ImGui::TableSetupColumn("Boot from", ImGuiTableColumnFlags_WidthFixed, 150.0f);
						ImGui::TableHeadersRow();

						for (size_t i = 0; i < files.size(); ++i)
						{
							ImGui::TableNextRow();

							// --- 1era Columna: Checkbox para incluir ---
							ImGui::TableSetColumnIndex(0);
							// Usamos un ID único por fila utilizando el índice o puntero
							std::string labelCheckbox = "##incluir" + std::to_string(i);

							bool includeValue = includeFlags[i];
							if (ImGui::Checkbox(labelCheckbox.c_str(), &includeValue))
							{
								includeFlags[i] = includeValue;
							}

							// --- 2da Columna: Nombre del archivo ---
							ImGui::TableSetColumnIndex(1);
							ImGui::Text("%s", files[i].c_str());

							// --- 3ra Columna: Radio button para el nivel de booteo ---
							ImGui::TableSetColumnIndex(2);
							// Si tienes niveles fijos (ej: Nivel 0, Nivel 1, Nivel 2) puedes iterar o pintarlos.
							// Aquí asumiremos 2 o 3 opciones globales de radio buttons asociadas a esta fila 
							// (o un grupo de radio buttons globales donde el archivo 'i' activo define el booteo).

							// Ejemplo: Si el radio button es exclusivo para TODO el build eligiendo cuál archivo es el de booteo:
							std::string labelRadio = "##" + std::to_string(i);
							if (ImGui::RadioButton(labelRadio.c_str(), bootLevelSelected == (int)i)) {
								bootLevelSelected = (int)i; // Al hacer clic, este archivo se convierte en el seleccionado
							}
						}

						ImGui::EndTable();
					}
				}
				ImGui::EndChild();
				ImGui::PopStyleVar(2);
			}
			else
			{
				ImGui::Text("Build progress:");

				// Creamos una caja de texto con scroll (Child Window) para los logs
				ImVec2 logSize(0, size.y - 30.0f);
				ImGui::BeginChild("LogScrollRegion", logSize, true, ImGuiWindowFlags_HorizontalScrollbar);

				// Obtenemos el texto actual del stream de forma segura y lo imprimimos
				std::string currentLogs = logStream.str();
				ImGui::TextUnformatted(currentLogs.c_str());

				// Opcional: Auto-scroll hacia abajo conforme llegan nuevos mensajes
				if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
					ImGui::SetScrollHereY(1.0f);

				ImGui::EndChild();
			}
		},
		canClick,
		[&]
		{
			if (!building)
			{
				StartBuildingProcess();
			}
			else if (completed)
			{
				showing = false;
			}
		}, building ? "Close" : "Build"
	);
}

std::vector<std::string> BuildModal::GetYamlFiles(const std::string& rutaDirectorio)
{
	std::vector<std::string> archivosYaml;

	if (!fs::exists(rutaDirectorio) || !fs::is_directory(rutaDirectorio)) {
		return archivosYaml;
	}

	for (const auto& entrada : fs::directory_iterator(rutaDirectorio)) {
		if (entrada.is_regular_file()) {
			std::string ext = entrada.path().extension().string();

			if (ext == ".yaml" || ext == ".yml") {
				archivosYaml.push_back(entrada.path().filename().string());
			}
		}
	}

	return archivosYaml;
}

std::unordered_map<SceneObjectType, std::function<void(nlohmann::json& sceneObject, std::set<std::filesystem::path>& files, std::set<JUUID>& templates, ThreadSafeStream& logStream)>> filesGatherer =
{
	{ SO_Renderables, [](nlohmann::json& sceneObject, std::set<std::filesystem::path>& files, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		Renderable::GatherFiles(sceneObject, files, templates, logStream);
	}
	},
	{ SO_Lights, [](nlohmann::json& sceneObject, std::set<std::filesystem::path>& files, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		Light::GatherFiles(sceneObject, files, templates, logStream);
	}
	},
	{ SO_Cameras, [](nlohmann::json& sceneObject, std::set<std::filesystem::path>& files, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		Camera::GatherFiles(sceneObject, files, templates, logStream);
	}
	},
	{ SO_SoundEffects, [](nlohmann::json& sceneObject, std::set<std::filesystem::path>& files, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		SoundFX::GatherFiles(sceneObject, files, templates, logStream);
	}
	},
	{ SO_PhysicScenes, [](nlohmann::json& sceneObject, std::set<std::filesystem::path>& files, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		PhysicScene::GatherFiles(sceneObject, files, templates, logStream);
	}
	},
	{ SO_Triggers, [](nlohmann::json& sceneObject, std::set<std::filesystem::path>& files, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		Trigger::GatherFiles(sceneObject, files, templates, logStream);
	}
	},
	{ SO_Boundaries, [](nlohmann::json& sceneObject, std::set<std::filesystem::path>& files, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		Boundary::GatherFiles(sceneObject, files, templates, logStream);
	}
	},
	{ SO_SceneControllers, [](nlohmann::json& sceneObject, std::set<std::filesystem::path>& files, std::set<JUUID>& templates, ThreadSafeStream& logStream)
	{
		SceneController::GatherFiles(sceneObject, files, templates, logStream);
	}
	},
};

void BuildModal::GatherFilesFromLevel(nlohmann::json& level, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
{
	//get the files directly used by the scene objects
	for (auto& [sceneObjectType, gatherer] : filesGatherer)
	{
		if (!level.contains(SceneObjectTypeJsonContainer.at(sceneObjectType)))
			continue;

		nlohmann::json& typeArr = level.at(SceneObjectTypeJsonContainer.at(sceneObjectType));
		for (unsigned int i = 0; i < typeArr.size(); i++)
		{
			gatherer(typeArr.at(i), filesToCopy, templates, logStream);
		}
	}
}

void BuildModal::GatherRecursiveFilesFromFolder(const std::filesystem::path& folderPath, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
{
	if (!std::filesystem::exists(folderPath) || !std::filesystem::is_directory(folderPath))
		return;

	// copy recursive
	for (const auto& entry : std::filesystem::recursive_directory_iterator(folderPath))
	{
		if (!entry.is_regular_file())
			continue;

		std::filesystem::path filePath = entry.path();

		if (filesToCopy.contains(filePath))
			continue;

		auto [iter, inserted] = filesToCopy.insert(filePath);
		if (inserted)
			logStream << "resource: " << filePath.string() << "\n";
	}
}

void BuildModal::GatherExecutablesFromFolder(const std::filesystem::path& folderPath, std::set<std::filesystem::path>& filesToCopy, ThreadSafeStream& logStream)
{
	if (!std::filesystem::exists(folderPath) || !std::filesystem::is_directory(folderPath))
	{
		logStream << "[Warning] invalid folder: " << folderPath.string() << "\n";
		return;
	}

	//do not recursive iterate
	for (const auto& entry : std::filesystem::directory_iterator(folderPath))
	{
		if (!entry.is_regular_file())
			continue;

		std::filesystem::path filePath = entry.path();

		if (filePath.extension() == ".exe")
		{
			if (filesToCopy.contains(filePath))
				continue;

			auto [iter, inserted] = filesToCopy.insert(filePath);
			if (inserted)
			{
				logStream << "Copied: " << filePath.string() << "\n";
			}
		}
	}
}

void BuildModal::CopyFilesFromTargetFolderToBuildFolder(
	const std::set<std::filesystem::path>& filesToCopy,
	ThreadSafeStream& logStream)
{
	std::filesystem::path buildDir = "../Build";

	if (std::filesystem::exists(buildDir))
	{
		logStream << "[Cleanup] Build folder\n";
		try
		{
			std::filesystem::remove_all(buildDir);
			logStream << "[Cleanup] success\n";
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			logStream << "[Error] cannot delete Build folder: " << e.what() << "\n";
			return; // Si no se puede borrar, es arriesgado continuar copiando
		}
	}

	logStream << "[Build] Starting files copy\n";

	for (const auto& srcPath : filesToCopy)
	{
		try
		{
			if (!std::filesystem::exists(srcPath))
			{
				logStream << "[Warning] missing file: " << srcPath.string() << "\n";
				continue;
			}

			std::filesystem::path relativePath = srcPath;
			std::filesystem::path destPath = buildDir / relativePath;
			std::filesystem::create_directories(destPath.parent_path());
			std::filesystem::copy_file(srcPath, destPath, std::filesystem::copy_options::overwrite_existing);

			logStream << "Copied: " << destPath.string() << "\n";
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			logStream << "[Copy error] " << e.what() << "\n";
		}
	}

	logStream << "[Build] Files copy finished\n";
}

void BuildModal::CopyFilesIntoBuildFolder(const std::set<std::filesystem::path>& filesToCopy, ThreadSafeStream& logStream)
{
	std::filesystem::path buildFolderPath = "../Build";

	for (const auto& srcPath : filesToCopy)
	{
		try
		{
			if (!std::filesystem::exists(srcPath))
			{
				continue;
			}

			std::filesystem::path destPath = buildFolderPath / srcPath.filename();
			std::filesystem::copy_file(srcPath, destPath, std::filesystem::copy_options::overwrite_existing);
			logStream << "Copied: " << destPath.filename().string() << "\n";
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			logStream << "[Copy error] " << e.what() << "\n";
		}
	}
}

void BuildModal::WriteBootLevelIntoBuildFolder(const std::string& bootLevelName, ThreadSafeStream& logStream)
{
	std::filesystem::path buildFolderPath = "../Build";
	std::filesystem::path bootIniPath = buildFolderPath / "boot.ini";

	// Abrir el archivo de salida (si no existe se crea, si existe se sobrescribe)
	std::ofstream outFile(bootIniPath);
	if (outFile.is_open())
	{
		outFile << bootLevelName;
		outFile.close();
		logStream << "[Build]boot.ini created: " << bootLevelName << "\n";
	}
	else
	{
		logStream << "[Error] cannot write boot.ini at: " << bootIniPath.string() << "\n";
	}
}

void BuildModal::StartBuildingProcess()
{
	std::thread levelThread([](BuildModal* modal)
		{
			using namespace Scene::Level;

			modal->building = true;
			modal->cancelable = false;
			modal->logStream.clear();

			std::set<std::filesystem::path> filesToCopy;
			std::set<JUUID> templates;

			for (size_t idx = 0ULL; idx < modal->files.size(); idx++)
			{
				if (!modal->includeFlags.at(idx))
					continue;

				std::string& file = modal->files.at(idx);
				filesToCopy.insert(defaultLevelsFolder + file);

				//load the level
				nlohmann::json lvl = GetLevelFromFile(file);

				GatherFilesFromLevel(lvl, filesToCopy, templates, modal->logStream);
			}

			std::set<std::filesystem::path> recursive_folders = { defaultScriptsFolder, defaultShadersFolder };
			for (auto& p : recursive_folders)
			{
				GatherRecursiveFilesFromFolder(p, filesToCopy, templates, modal->logStream);
			}
			std::set<std::filesystem::path> fixed_asset_files =
			{
				defaultUIFolder + "cacert.pem",
				defaultUIFolder + "icudt67l.dat",
				defaultTemplatesFolder + Templates::Shader::templateName,
				defaultTemplatesFolder + Templates::Material::templateName,
				defaultTemplatesFolder + Templates::Model3D::templateName,
				defaultTemplatesFolder + Templates::RenderPass::templateName,
				defaultTemplatesFolder + Templates::Sound::templateName,
				defaultTemplatesFolder + Templates::Texture::templateName,
				defaultTemplatesFolder + Templates::PhysicGeometry::templateName,
				defaultTemplatesFolder + Templates::HtmlUI::templateName,
				defaultTemplatesFolder + Templates::Mold::templateName,
			};
			for (auto& p : fixed_asset_files)
			{
				filesToCopy.insert(p);
				modal->logStream << "resource: " << p.string() << "\n";
			}

			std::set<std::filesystem::path> release_files =
			{
				"../Release/AppCore.dll",
				"../Release/assimp-vc143-mt.dll",
				"../Release/dxcompiler.dll",
				"../Release/dxil.dll",
				"../Release/icudtl.dat",
				"../Release/icui18n.dll",
				"../Release/icuuc.dll",
				"../Release/PhysXCommon_64.dll",
				"../Release/PhysXCooking_64.dll",
				"../Release/PhysXFoundation_64.dll",
				"../Release/PhysXGpu_64.dll",
				"../Release/PhysX_64.dll",
				"../Release/Ultralight.dll",
				"../Release/UltralightCore.dll",
				"../Release/v8.dll",
				"../Release/v8pp.dll",
				"../Release/v8_libbase.dll",
				"../Release/v8_libplatform.dll",
				"../Release/WebCore.dll",
				"../Release/zlib.dll",
			};

			GatherExecutablesFromFolder("../Release", release_files, modal->logStream);

			CopyFilesFromTargetFolderToBuildFolder(filesToCopy, modal->logStream);
			CopyFilesIntoBuildFolder(release_files, modal->logStream);

			WriteBootLevelIntoBuildFolder(modal->GetBootLevelName(), modal->logStream);
			modal->completed = true;
		}, this
	);
	levelThread.detach();
}

std::string BuildModal::GetBootLevelName() const {
	if (bootLevelSelected >= 0 && bootLevelSelected < static_cast<int>(files.size())) {
		return files[bootLevelSelected];
	}
	return "";
}
