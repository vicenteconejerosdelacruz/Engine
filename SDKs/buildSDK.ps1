$sdkfolder=&"pwd"
$msbuild = &"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe

function DownloadNuget {
	cd $sdkfolder
	if (-not (Test-Path ".\nuget.exe")) {
		Invoke-WebRequest -Uri "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe" -OutFile ".\nuget.exe"
	}
}

function CheckUltraLight {
	$ultralightCheck = "ultralight\CMakeLists.txt"

	cd $sdkfolder
	if (-not (Test-Path $ultralightCheck)) {
		Write-Host "Error: ultralight may not be present. please verify(read version.txt)" -ForegroundColor Red
		exit 1
	}
}

function Buildv8pp {
	cd $sdkfolder
	cd "v8pp-2.1.1"
	$packages = @(
		@{ Id = "v8-v142-x64"; Version = "8.0.426.28" },
		@{ Id = "v8-v142-x86"; Version = "7.5.288.23" },
		@{ Id = "v8-v143-x64"; Version = "9.8.177.4" },
		@{ Id = "v8-v143-x86"; Version = "9.8.177.4" },
		@{ Id = "v8.redist-v142-x64"; Version = "8.0.426.28" },
		@{ Id = "v8.redist-v142-x86"; Version = "7.5.288.23" },
		@{ Id = "v8.redist-v143-x64"; Version = "9.8.177.4" },
		@{ Id = "v8.redist-v143-x86"; Version = "9.8.177.4" },
		@{ Id = "v8.symbols-v142-x64"; Version = "8.0.426.28" },
		@{ Id = "v8.symbols-v142-x86"; Version = "7.5.288.23" }
	)

	foreach ($pkg in $packages) {
		Write-Host "Descargando $($pkg.Id) versión $($pkg.Version)..." -ForegroundColor Cyan
		../nuget.exe install $pkg.Id -Version $pkg.Version -OutputDirectory "packages"
	}

	Write-Host "¡Todos los paquetes han sido descargados en la carpeta packages!" -ForegroundColor Green

	$filePath = "v8pp/utility.hpp"

	if (Test-Path $filePath) {
		$lines = Get-Content -Path $filePath
		
		# Verificar si alguna línea ya incluye <string>
		$hasStringInclude = $false
		foreach ($line in $lines) {
			if ($line.Trim() -eq "#include <string>") {
				$hasStringInclude = $true
				break
			}
		}

		if ($hasStringInclude) {
			Write-Host "El archivo ya contiene '#include <string>'." -ForegroundColor Cyan
		} else {
			Write-Host "No se encontró '#include <string>'. Agregándolo en la línea 4..." -ForegroundColor Yellow
			
			# Asegurarse de que el archivo tenga al menos 3 líneas antes de insertar en la 4
			if ($lines.Count -ge 3) {
				# Insertar en la posición 4 (índice 3 en arrays basados en 0)
				$newLines = $lines[0..2] + "#include <string>" + $lines[3..($lines.Count - 1)]
			} else {
				# Si el archivo es muy corto, simplemente lo añadimos al final o donde corresponda
				$newLines = $lines + "#include <string>"
			}

			Set-Content -Path $filePath -Value $newLines
			Write-Host "¡Línea añadida con éxito en $filePath!" -ForegroundColor Green
		}
	} else {
		Write-Error "No se encontró el archivo en la ruta: $filePath"
	}

	New-Item -ItemType Directory -Force -Path "out"
	cd out
	cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON ..

	& $msbuild v8pp.sln /p:Configuration=Release /t:Rebuild
	& $msbuild v8pp.sln /p:Configuration=Debug /t:Rebuild
}

function BuildAssimp {
	cd "assimp\assimp-6.0.5"
	cmake -B build -S . `
	-DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF `
	-DASSIMP_BUILD_GLTF_IMPORTER=ON `
	-DASSIMP_NO_EXPORT=ON `
	-DASSIMP_BUILD_TESTS=OFF `
	-DASSIMP_BUILD_ASSIMP_TOOLS=OFF `
	-DBUILD_SHARED_LIBS=ON
	cmake --build build --config Release
	cmake --build build --config Debug
}

function BuildDirectXTex {
	cd $sdkfolder
	cd DirectXTex
	& $msbuild ./DirectXTex_Desktop_2022.sln /p:Configuration=ReleaseLib /t:Rebuild
	& $msbuild ./DirectXTex_Desktop_2022.sln /p:Configuration=DebugLib /t:Rebuild
}

function BuildDirectXTK12 {
	cd $sdkfolder
	cd DirectXTK12
	& $msbuild DirectXTK_Desktop_2022_Win10.sln /p:Configuration=Release /t:Rebuild
	& $msbuild DirectXTK_Desktop_2022_Win10.sln /p:Configuration=Debug /t:Rebuild
}

function BuildImGui {
	cd $sdkfolder
	cd imgui
	& $msbuild imgui.sln /p:Configuration=Release /t:Rebuild
	& $msbuild imgui.sln /p:Configuration=Debug /t:Rebuild
}

function BuildImGuizmo {
	cd $sdkfolder
	cd imguizmo
	& $msbuild ImGuizmo.sln /p:Configuration=Release /t:Rebuild
	& $msbuild ImGuizmo.sln /p:Configuration=Debug /t:Rebuild
}

function BuildYamlCpp {
	cd $sdkfolder
	cd "yaml-cpp-yaml-cpp-0.9.0"
	New-Item -ItemType Directory -Force -Path "build"
	cd build
	cmake ..
	& $msbuild YAML_CPP.sln /p:Configuration=Release /t:Rebuild
	& $msbuild YAML_CPP.sln /p:Configuration=Debug /t:Rebuild
}

CheckUltraLight
DownloadNuget
Buildv8pp
BuildAssimp
BuildDirectXTex
BuildDirectXTK12
BuildImGui
BuildImGuizmo
BuildYamlCpp

cd $sdkfolder

#copy ./assimp/assimp-6.0.4/build/bin/Debug/assimp-vc143-mtd.dll ../Engine/Target
#copy ./assimp/assimp-6.0.4/build/bin/Release/assimp-vc143-mt.dll ../Engine/Target