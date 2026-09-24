#pragma once

#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#if defined(_EDITOR)

class CommandLine
{
public:
	CommandLine()
	{
		Parse();
	}

	// Getters de estado
	bool IsGenerateDDS() const { return m_generateDDS; }
	bool IsGenerateBuild() const { return HasBuildParams() && HasBootParam(); }
	bool IsValid() const { return m_isValid; }
	bool HasBuildParams() const { return !m_buildParams.empty(); }
	bool HasBootParam() const { return !m_bootParam.empty(); }

	const std::vector<std::string>& GetBuildParams() const { return m_buildParams; }
	const std::string& GetBootParam() const { return m_bootParam; }

private:

	bool m_generateDDS = false;
	bool m_isValid = true;
	std::vector<std::string> m_buildParams;
	std::string m_bootParam;

	void Parse()
	{
		int argc = 0;
		LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

		if (!argv) return;

		for (int i = 0; i < argc; i++)
		{
			std::string arg = nostd::WStringToString(argv[i]);

			if (arg == "--generate-dds")
			{
				m_generateDDS = true;
			}
			else if (arg.rfind("--build=", 0) == 0)
			{
				std::string value = arg.substr(8);
				std::stringstream ss(value);
				std::string item;
				while (std::getline(ss, item, ','))
				{
					if (!item.empty()) m_buildParams.push_back(item);
				}
			}
			else if (arg.rfind("--boot=", 0) == 0)
			{
				m_bootParam = arg.substr(7);
			}
		}

		// Liberación automática al terminar el parseo
		LocalFree(argv);

		// Validaciones y acople de consola
		ValidateAndSetupConsole();
	}

	void ValidateAndSetupConsole()
	{
		bool hasOtherFlags = HasBuildParams() || HasBootParam();

		// Si se recibió alguna bandera especial, adjuntamos la consola
		if (m_generateDDS || hasOtherFlags)
		{
			if (AttachConsole(ATTACH_PARENT_PROCESS))
			{
				FILE* fp;
				freopen_s(&fp, "CONOUT$", "w", stdout);
				freopen_s(&fp, "CONOUT$", "w", stderr);
			}
		}

		// Regla 1: No mezclar --generate-dds con otras opciones
		if (m_generateDDS && hasOtherFlags)
		{
			std::cerr << "[ERROR] La opcion --generate-dds no se puede combinar con otros argumentos (--build o --boot)." << std::endl;
			m_isValid = false;
			return;
		}

		// Regla 2: Dependencia mutua entre --build y --boot
		if (HasBuildParams() != HasBootParam())
		{
			std::cerr << "[ERROR] Los argumentos --build y --boot deben usarse juntos. Falta uno de los dos." << std::endl;
			m_isValid = false;
			return;
		}

		// Regla 3: Si ambos están definidos, --boot debe estar contenido dentro de --build
		if (HasBuildParams() && HasBootParam())
		{
			auto it = std::find(m_buildParams.begin(), m_buildParams.end(), m_bootParam);
			if (it == m_buildParams.end())
			{
				std::cerr << "[ERROR] El nivel especificado en --boot ('" << m_bootParam
					<< "') debe estar incluido en la lista de --build." << std::endl;
				m_isValid = false;
				return;
			}
		}
	}
};

#endif