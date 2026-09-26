$sdkfolder=&"pwd"

$msbuild = &"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe
$msbuild2022 = "C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\MSBuild.exe"

$project = "TestGame"

$debug_projects = @("Debug", "Editor_Debug")
$release_projects = @("Release", "Development", "Editor_Release", "Editor_Development")
$profile_projects = @("Debug", "Editor_Debug", "Development", "Editor_Release", "Editor_Development")

$assimpUrl = "https://github.com/assimp/assimp/archive/refs/tags/v6.0.5.zip"
$physxUrl = "https://github.com/NVIDIA-Omniverse/PhysX/archive/refs/tags/107.3-omni-and-physx-5.6.1.zip"
$directxTK12Url = "https://github.com/microsoft/DirectXTK12/archive/refs/tags/may2026.zip"

$nugetUrl = "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
$ProgressPreference = 'SilentlyContinue'

function Download-Nuget {
	cd $sdkfolder
	if (-not (Test-Path ".\nuget.exe")) {
		Invoke-WebRequest -Uri $nugetUrl -OutFile ".\nuget.exe"
	}
}

function Check-MSbuild2022 
{
	cd $sdkfolder
	if (-not (Test-Path $msbuild2022)) {
		Write-Host "Error: visual studio 2022 is needed in order to compile physx" -ForegroundColor Red
		exit 1
	}
}

function Check-UltraLight {
	$ultralightCheck = "ultralight\ultralight-free-sdk-1.4.0-win-x64\CMakeLists.txt"

	cd $sdkfolder
	if (-not (Test-Path $ultralightCheck)) {
		Write-Host "Error: ultralight may not be present. please verify(read version.txt)" -ForegroundColor Red
		exit 1
	}
}

function Install-Assimp {
    # Definir rutas seguras basadas en la ubicación del script o directorio actual
    $baseDir = if ($PSScriptRoot) { $PSScriptRoot } else { (Get-Location).Path }
    $extractDir = Join-Path $baseDir "assimp"

    # Verificar si el directorio assimp ya existe
    if (Test-Path $extractDir) {
        Write-Host "The directory 'assimp' already exists. Skip." -ForegroundColor Yellow
        return
    }

    if ($global:sdkfolder) {
        cd $global:sdkfolder
    }

    $outputZip = Join-Path $baseDir "assimp_temp.zip"

    Write-Host "Downloading Assimp..." -ForegroundColor Cyan
    try {
        # Descargar el archivo zip
        Invoke-WebRequest -Uri $assimpUrl -OutFile $outputZip -UseBasicParsing
        
        # Crear el directorio de destino si no existe
        if (-not (Test-Path $extractDir)) {
            New-Item -ItemType Directory -Force -Path $extractDir | Out-Null
        }

        Write-Host "Descomprimiendo en $extractDir..." -ForegroundColor Cyan
        # Extraer el contenido
        Expand-Archive -Path $outputZip -DestinationPath $extractDir -Force

        Write-Host "¡Assimp descargado y descomprimido con éxito!" -ForegroundColor Green
    }
    catch {
        Write-Error "Ocurrió un error durante la descarga o extracción: $_"
    }
    finally {
        # Limpiar el archivo zip temporal si existe
        if (Test-Path $outputZip) {
            Remove-Item $outputZip -Force
        }
    }
}

function Install-PhysX {
    param(
        [string]$TargetDir = "PhysX"
    )

    if (-not (Test-Path $TargetDir)) {
        Write-Host "El directorio '$TargetDir' no existe. Iniciando instalación..." -ForegroundColor Cyan


		$url = $physxUrl
        $zipFile = "physx_temp.zip"
        $tempDir = "temp_extracted"

        Write-Host "Descargando archivo ZIP..." -ForegroundColor Cyan
        Invoke-WebRequest -Uri $url -OutFile $zipFile

        Write-Host "Descomprimiendo..." -ForegroundColor Cyan
        Expand-Archive -Path $zipFile -DestinationPath $tempDir -Force

        $innerFolder = Join-Path $tempDir "PhysX-107.3-omni-and-physx-5.6.1"
		
        if (Test-Path $innerFolder) {
            Move-Item -Path $innerFolder -Destination $TargetDir
            Write-Host "¡Carpeta '$TargetDir' creada con éxito!" -ForegroundColor Green
        } else {
            Write-Error "No se encontró la carpeta interna esperada dentro del ZIP."
        }
		
        # Limpieza de archivos temporales
        Write-Host "Limpiando archivos temporales..." -ForegroundColor Cyan
        Remove-Item -Force $zipFile -ErrorAction SilentlyContinue
        Remove-Item -Recurse -Force $tempDir -ErrorAction SilentlyContinue

        Write-Host "¡Proceso finalizado!" -ForegroundColor Green
    } else {
        Write-Host "El directorio '$TargetDir' ya existe. Omitiendo la instalación." -ForegroundColor Green
    }
}

function Install-DirectXTK12 {
    # Ir directamente a la carpeta raíz de los SDKs
    cd $sdkfolder

    $outputZip = "directxtk12_temp.zip"
    $extractedFolder = "DirectXTK12-may2026"
    $targetFolder = "DirectXTK12"

    # Verificar si ya existe la carpeta final
    if (Test-Path $targetFolder) {
        Write-Host "El directorio 'DirectXTK12' ya existe. Omitiendo descarga e instalación." -ForegroundColor Yellow
        return
    }

    Write-Host "Descargando DirectX Tool Kit for Direct3D 12..." -ForegroundColor Cyan
    try {
        # Descargar usando la variable externa $directxTK12Url
        Invoke-WebRequest -Uri $directxTK12Url -OutFile $outputZip -UseBasicParsing

        Write-Host "Descomprimiendo archivos..." -ForegroundColor Cyan
        Expand-Archive -Path $outputZip -DestinationPath "." -Force

        # Renombrar la carpeta descomprimida si existe
        if (Test-Path $extractedFolder) {
            if (Test-Path $targetFolder) {
                Remove-Item $targetFolder -Recurse -Force
            }
            Rename-Item -Path $extractedFolder -NewName "DirectXTK12"
            Write-Host "¡DirectXTK12 descargado y renombrado con éxito a 'DirectXTK12'!" -ForegroundColor Green
        } else {
            Write-Warning "No se encontró la carpeta esperada 'DirectXTK12-may2026' tras la extracción."
        }
    }
    catch {
        Write-Error "Ocurrió un error durante la descarga o extracción de DirectX TK 12: $_"
    }
    finally {
        # Limpiar el archivo zip temporal
        if (Test-Path $outputZip) {
            Remove-Item $outputZip -Force
        }
    }
}

function Build-v8pp {
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

function Build-Assimp {
	cd $sdkfolder
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

function Build-DirectXTex {
	cd $sdkfolder
	cd DirectXTex
	& $msbuild ./DirectXTex_Desktop_2022.sln /p:Configuration=ReleaseLib -p:Platform=x64 /t:Rebuild
	& $msbuild ./DirectXTex_Desktop_2022.sln /p:Configuration=DebugLib -p:Platform=x64 /t:Rebuild
}

function Build-DirectXTK12 {
	cd $sdkfolder
	cd DirectXTK12
	& $msbuild DirectXTK_Desktop_2026.slnx /p:Configuration=Debug -p:Platform=x64 /t:Rebuild
	& $msbuild DirectXTK_Desktop_2026.slnx /p:Configuration=Release -p:Platform=x64 /t:Rebuild
}

function Build-ImGui {
	cd $sdkfolder
	cd imgui
	& $msbuild imgui.sln /p:Configuration=Release /t:Rebuild
	& $msbuild imgui.sln /p:Configuration=Debug /t:Rebuild
}

function Build-ImGuizmo {
	cd $sdkfolder
	cd imguizmo
	& $msbuild ImGuizmo.sln /p:Configuration=Release /t:Rebuild
	& $msbuild ImGuizmo.sln /p:Configuration=Debug /t:Rebuild
}

function Build-YamlCpp {
	cd $sdkfolder
	cd "yaml-cpp-yaml-cpp-0.9.0"
	New-Item -ItemType Directory -Force -Path "build"
	cd build
	cmake ..
	& $msbuild YAML_CPP.sln /p:Configuration=Release /t:Rebuild
	& $msbuild YAML_CPP.sln /p:Configuration=Debug /t:Rebuild
}

function Set-PhysXDynamicCRT {
    
	$PresetPath = Join-Path $PSScriptRoot "PhysX\physx\buildtools\presets\public\vc17win64.xml"

    if (Test-Path $PresetPath) {
        Write-Host "Modificando preset en $PresetPath..." -ForegroundColor Cyan
        
        [xml]$xml = Get-Content -Path $PresetPath
        
        # Buscar el nodo cmakeSwitch con name="NV_USE_STATIC_WINCRT"
        $node = $xml.SelectSingleNode("//cmakeSwitch[@name='NV_USE_STATIC_WINCRT']")
        
        if ($node) {
            if ($node.value -eq "True") {
                $node.value = "False"
                $xml.Save($PresetPath)
                Write-Host "¡Éxito! NV_USE_STATIC_WINCRT cambiado a 'False'." -ForegroundColor Green
            } else {
                Write-Host "El valor ya era 'False'. No se realizaron cambios." -ForegroundColor Yellow
            }
        } else {
            Write-Warning "No se encontró el nodo 'NV_USE_STATIC_WINCRT' en el archivo XML."
        }
    } else {
        Write-Error "No se encontró el archivo de preset en la ruta: $PresetPath"
    }
}

function Build-PhysX {
	cd $sdkfolder
	cd "PhysX\physx"
	Set-PhysXDynamicCRT
	"4" | .\generate_projects.bat
	& $msbuild2022 "compiler\vc17win64\PhysXSDK.sln" -p:Configuration=debug -p:Platform=x64 /t:Rebuild
	#& $msbuild2022 "compiler\vc17win64\PhysXSDK.sln" -p:Configuration=checked -p:Platform=x64 /t:Rebuild
	& $msbuild2022 "compiler\vc17win64\PhysXSDK.sln" -p:Configuration=profile -p:Platform=x64 /t:Rebuild
	& $msbuild2022 "compiler\vc17win64\PhysXSDK.sln" -p:Configuration=release -p:Platform=x64 /t:Rebuild
}

function Copy-Base-Dlls {
	cd $sdkfolder
	
	$files = @(
        "dxc\dxc_2024_07_31\bin\x64\dxcompiler.dll",
        "dxc\dxc_2024_07_31\bin\x64\dxil.dll",
        "ultralight\ultralight-free-sdk-1.4.0-win-x64\bin\AppCore.dll",
        "ultralight\ultralight-free-sdk-1.4.0-win-x64\bin\Ultralight.dll",
        "ultralight\ultralight-free-sdk-1.4.0-win-x64\bin\UltralightCore.dll",
        "ultralight\ultralight-free-sdk-1.4.0-win-x64\bin\WebCore.dll"
    )

	$targetFolder = "..\TestGame\Target\"
	
	foreach ($file in $files) {
        if (Test-Path $file) {
            Write-Host "Copy: $file" -ForegroundColor Cyan
            Copy-Item -Path $file -Destination $targetFolder -Force
        } else {
            Write-Host "Warning: '$file' Not found" -ForegroundColor Yellow
        }
    }
}

function Create-Output-Folders {
	cd $sdkfolder
	
	cd ../$project
	./createOutputFolders.ps1
}
	
function Copy-Debug-Resources {
    param(
        [string[]]$Directories
    )
    
    $files = @(
        "assimp\assimp-6.0.5\build\bin\Debug\assimp-vc143-mtd.dll",
        "v8pp-2.1.1\out\v8.dll",
        "v8pp-2.1.1\out\v8_libbase.dll",
        "v8pp-2.1.1\out\v8_libplatform.dll",
        "v8pp-2.1.1\out\zlib.dll",
        "v8pp-2.1.1\out\Debug\v8pp.dll",
        "v8pp-2.1.1\out\icuuc.dll",
        "v8pp-2.1.1\out\icudtl.dat",
        "v8pp-2.1.1\out\icui18n.dll"
    )
    
	Set-Location $sdkfolder

	foreach ($dir in $Directories) {
		Write-Host "Procesando directorio de destino: $dir" -ForegroundColor Cyan
		
		# Crear el directorio de destino si no existe
		if (-not (Test-Path ../$project/$dir)) {
			New-Item -ItemType Directory -Force -Path ../$project/$dir | Out-Null
		}

		foreach ($file in $files) {
			if (Test-Path $file) {
				Copy-Item -Path $file -Destination ../$project/$dir -Force
			} else {
				Write-Host "  Advertencia: No se encontró el archivo fuente '$file'" -ForegroundColor Yellow
			}
		}
	}
}

function Copy-Release-Resources {
	param(
        [string[]]$Directories
    )
	
    $files = @(
        "assimp\assimp-6.0.5\build\bin\Release\assimp-vc143-mt.dll",
        "v8pp-2.1.1\out\v8.dll",
        "v8pp-2.1.1\out\v8_libbase.dll",
        "v8pp-2.1.1\out\v8_libplatform.dll",
        "v8pp-2.1.1\out\zlib.dll",
        "v8pp-2.1.1\out\Release\v8pp.dll",
        "v8pp-2.1.1\out\icuuc.dll",
        "v8pp-2.1.1\out\icudtl.dat",
        "v8pp-2.1.1\out\icui18n.dll"
    )
    
	Set-Location $sdkfolder

	foreach ($dir in $Directories) {
		Write-Host "Procesando directorio de destino: $dir" -ForegroundColor Cyan
		
		# Crear el directorio de destino si no existe
		if (-not (Test-Path ../$project/$dir)) {
			New-Item -ItemType Directory -Force -Path ../$project/$dir | Out-Null
		}

		foreach ($file in $files) {
			if (Test-Path $file) {
				Copy-Item -Path $file -Destination ../$project/$dir -Force
			} else {
				Write-Host "  Advertencia: No se encontró el archivo fuente '$file'" -ForegroundColor Yellow
			}
		}
	}
}

function Copy-PhysXBinaries {
    # Definir rutas base de forma segura
    $baseDir = if ($PSScriptRoot) { $PSScriptRoot } else { (Get-Location).Path }
    $physxBinBase = Join-Path $baseDir "PhysX\physx\bin\win.x86_64.vc143.md"

    # Lista exacta de DLLs permitidas
    $allowedDlls = @(
        "PhysXCommon_64.dll",
        "PhysXCooking_64.dll",
        "PhysXFoundation_64.dll",
        "PhysXGpu_64.dll",
        "PhysX_64.dll"
    )

    # Grupos de proyectos definidos
    $physx_debug_projects   = @("Debug", "Editor_Debug")
    $physx_profile_projects = @("Development", "Editor_Development")
    $physx_release_projects = @("Release", "Editor_Release")

    # Función interna para realizar la copia filtrada de las DLLs
    function Invoke-DllCopy {
        param([string]$SourcePath, [string]$TargetProject)
        
        # Ruta de destino: ..\$TargetProject (un nivel arriba, luego el nombre del proyecto/configuración)
        $destDir = Join-Path $baseDir "..\$project\$TargetProject"
        
        if (Test-Path $SourcePath) {
            if (-not (Test-Path $destDir)) {
                New-Item -ItemType Directory -Force -Path $destDir | Out-Null
            }
            Write-Host "Copiando DLLs esenciales de PhysX hacia [$destDir]..." -ForegroundColor Cyan
            
            foreach ($dll in $allowedDlls) {
                $sourceFile = Join-Path $SourcePath $dll
                if (Test-Path $sourceFile) {
                    Copy-Item -Path $sourceFile -Destination $destDir -Force
                } else {
                    Write-Warning "No se encontró la DLL esperada: $dll en $SourcePath"
                }
            }
        } else {
            Write-Warning "La ruta de origen no existe: $SourcePath"
        }
    }

    # 1. Procesar Debug
    $debugSrc = Join-Path $physxBinBase "debug"
    foreach ($proj in $physx_debug_projects) {
        Invoke-DllCopy -SourcePath $debugSrc -TargetProject $proj
    }

    # 2. Procesar Profile (Development)
    $profileSrc = Join-Path $physxBinBase "profile"
    foreach ($proj in $physx_profile_projects) {
        Invoke-DllCopy -SourcePath $profileSrc -TargetProject $proj
    }

    # 3. Procesar Release
    $releaseSrc = Join-Path $physxBinBase "release"
    foreach ($proj in $physx_release_projects) {
        Invoke-DllCopy -SourcePath $releaseSrc -TargetProject $proj
    }

    Write-Host "¡Las DLLs esenciales de PhysX han sido copiadas con éxito!" -ForegroundColor Green
}

function Copy-PixRuntimeDll {
    param(
        [string[]]$Directories
    )

    $files = @(
        "WinPixEventRuntime\bin\x64\WinPixEventRuntime.dll"
    )

    Set-Location $sdkfolder

    foreach ($dir in $Directories) {
        Write-Host "Procesando directorio de destino: $dir" -ForegroundColor Cyan
        
        # Crear el directorio de destino si no existe
        if (-not (Test-Path ../$project/$dir)) {
            New-Item -ItemType Directory -Force -Path ../$project/$dir | Out-Null
        }

        foreach ($file in $files) {
            if (Test-Path $file) {
                Copy-Item -Path $file -Destination ../$project/$dir -Force
            } else {
                Write-Host "  Advertencia: No se encontró el archivo fuente '$file'" -ForegroundColor Yellow
            }
        }
    }
}

#make sure we have nuget
Download-Nuget

#check needed libraries and msbuild2022
Check-UltraLight
Check-MSbuild2022

#download dependencies
Install-Assimp
Install-PhysX
Install-DirectXTK12

#build the downloaded dependencies
Build-Assimp
Build-PhysX
Build-DirectXTK12

#build the libraries we have in the repo
Build-DirectXTex
Build-v8pp
Build-ImGui
Build-ImGuizmo
Build-YamlCpp

#copy the common dlls to Target
Copy-Base-Dlls

#create the symlinks for the different versions of the project
Create-Output-Folders

#copy the dlls to each environment
Copy-Debug-Resources $debug_projects
Copy-Release-Resources $release_projects
Copy-PhysXBinaries
Copy-PixRuntimeDll $profile_projects

cd $sdkfolder
