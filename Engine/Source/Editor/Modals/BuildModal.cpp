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
#include "../Builder/BuildMaker.h"

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

void BuildModal::StartBuildingProcess()
{
	std::set<std::string> filesToBuild;
	for (size_t idx = 0ULL; idx < files.size(); idx++)
	{
		if (!includeFlags.at(idx))
			continue;

		filesToBuild.insert(files.at(idx));
	}

	std::thread levelThread = CreateBuilderThread(
		[=]
		{
			return filesToBuild;
		},
		[&]
		{
			return GetBootLevelName();
		},
		[&]
		{
			building = true;
			cancelable = false;
			logStream.clear();
		},
		[&]
		{
			completed = true;
		},
		&logStream
	);
	levelThread.detach();
}

std::string BuildModal::GetBootLevelName() const {
	if (bootLevelSelected >= 0 && bootLevelSelected < static_cast<int>(files.size())) {
		return files[bootLevelSelected];
	}
	return "";
}
