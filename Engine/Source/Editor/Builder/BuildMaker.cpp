#include "pch.h"
#include "BuildMaker.h"
#include <Level.h>

void GenerateBuild(const std::vector<std::string>& buildParams, const std::string& bootParam)
{
	ThreadSafeStream logStream;
	std::set<std::string> filesToBuild;
	for (size_t idx = 0ULL; idx < buildParams.size(); idx++)
	{
		filesToBuild.insert(buildParams.at(idx));
	}

	auto getFileList = [=] { return filesToBuild; };
	auto getBootLevel = [=] { return bootParam; };
	auto onStart = [&] { logStream.clear(); };
	auto onComplete = [&] {};

	// Flag para controlar cuándo debe terminar el hilo de monitoreo
	std::atomic<bool> isBuilding{ true };

	// 1. Lanzamos el hilo que lee e imprime el logStream en tiempo real
	std::thread loggerThread([&logStream, &isBuilding]()
		{
			size_t lastPosition = 0;

			// Mientas el builder esté activo O queden caracteres por imprimir
			while (isBuilding.load() || lastPosition < logStream.str().size())
			{
				std::string currentLog = logStream.str();

				if (currentLog.size() > lastPosition)
				{
					// Extrae e imprime únicamente el texto nuevo
					std::string newChunk = currentLog.substr(lastPosition);
					std::cout << newChunk << std::flush;
					OutputDebugStringA(newChunk.c_str());
					lastPosition = currentLog.size();
				}

				// Evita consumo excesivo de CPU entre revisiones
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
		}
	);

	// 2. Lanzamos el hilo del trabajo principal (CreateBuilderThread)
	std::thread levelThread = CreateBuilderThread(getFileList, getBootLevel, onStart, onComplete, &logStream);

	// 3. El hilo principal se bloquea aquí esperando al builder
	levelThread.join();

	// 4. Notificamos al loggerThread que el builder terminó
	isBuilding.store(false);

	// 5. Esperamos a que loggerThread imprima lo último que haya quedado pendiente y finalice
	loggerThread.join();
}

std::thread CreateBuilderThread(std::function<std::set<std::string>()> getFileList, std::function<std::string()> getBootLevel,
	std::function<void()> onStart, std::function<void()> onComplete, ThreadSafeStream* logStream)
{
	std::thread levelThread([](std::function<std::set<std::string>()> getFileList, std::function<std::string()> getBootLevel,
		std::function<void()> onStart, std::function<void()> onComplete, ThreadSafeStream* logStream)
		{
			using namespace Scene::Level;

			onStart();

			std::set<std::filesystem::path> filesToCopy;
			std::set<JUUID> templates;

			std::set<std::string> files = getFileList();
			std::for_each(files.begin(), files.end(), [&](auto file)
				{
					std::filesystem::path fpath = file;
					if (!fpath.has_extension())
					{
						fpath.replace_extension(".yaml");
					}
					filesToCopy.insert(defaultLevelsFolder + fpath.string());

					//load the level
					nlohmann::json lvl = GetLevelFromFile(file);

					GatherFilesFromLevel(lvl, filesToCopy, templates, *logStream);
				}
			);

			std::set<std::filesystem::path> recursive_folders = { defaultScriptsFolder, defaultShadersFolder };
			for (auto& p : recursive_folders)
			{
				GatherRecursiveFilesFromFolder(p, filesToCopy, templates, *logStream);
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
				*logStream << "resource: " << p.string() << "\n";
			}

			std::set<std::filesystem::path> release_files = {
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

			GatherExecutablesFromFolder("../Release", release_files, *logStream);
			CopyFilesFromTargetFolderToBuildFolder(filesToCopy, *logStream);
			CopyFilesIntoBuildFolder(release_files, *logStream);
			WriteBootLevelIntoBuildFolder(getBootLevel(), *logStream);

			onComplete();

		}, getFileList, getBootLevel, onStart, onComplete, logStream

	);

	return levelThread;
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

void GatherFilesFromLevel(nlohmann::json& level, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
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

void GatherRecursiveFilesFromFolder(const std::filesystem::path& folderPath, std::set<std::filesystem::path>& filesToCopy, std::set<JUUID>& templates, ThreadSafeStream& logStream)
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

void GatherExecutablesFromFolder(const std::filesystem::path& folderPath, std::set<std::filesystem::path>& filesToCopy, ThreadSafeStream& logStream)
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

void CopyFilesFromTargetFolderToBuildFolder(
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

void CopyFilesIntoBuildFolder(const std::set<std::filesystem::path>& filesToCopy, ThreadSafeStream& logStream)
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

void WriteBootLevelIntoBuildFolder(const std::string& bootLevelName, ThreadSafeStream& logStream)
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