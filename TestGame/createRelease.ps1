$srcfolder = "Target"
$releasefolder = "Release"
$intermediatefolder = "Build"
# Generar timestamp único (AñoMesDía_HoraMinutoSegundo)
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$zipOutput = "Release_$timestamp.zip"

function CreateRelease
{
    param (
        [string]$Src,
        [string]$Release,
        [string]$Intermediate
    )

    # Extensiones y directorios de depuración/temporales de Visual Studio a ignorar
    $debugExtensions = @('.pdb', '.ilk', '.rsp', '.pgd', '.pgc', '.exp', '.iobj', '.ipdb', '.idb')
    $debugFolders    = @('.vs', 'ipch', '$Recycle.Bin')

    # Convertir rutas relativas a rutas absolutas del sistema de archivos
    $baseReleasePath = (Get-Item -Path $Release).FullName.TrimEnd('\')
    $baseSrcPath     = (Get-Item -Path $Src).FullName.TrimEnd('\')
    
    # Si Build no existe, se crea primero
    if (-not (Test-Path -Path $Intermediate)) {
        New-Item -ItemType Directory -Path $Intermediate -Force | Out-Null
    }
    $baseBuildPath = (Get-Item -Path $Intermediate).FullName.TrimEnd('\')

    # Función interna para determinar si un elemento es de debugging
    function Test-IsDebugItem {
        param([System.IO.FileSystemInfo]$Item)

        # 1. Ignorar por extensión de archivo
        if (-not $Item.PSIsContainer -and $debugExtensions -contains $Item.Extension.ToLower()) {
            return $true
        }

        # 2. Ignorar si la ruta contiene alguna carpeta de debug (.vs, ipch, etc.)
        foreach ($folder in $debugFolders) {
            if ($Item.FullName -match "\\$([regex]::Escape($folder))(\\|$)") {
                return $true
            }
        }

        return $false
    }

    # 1. Recrear la estructura de directorios reales (no symlinks) en Build
    Get-ChildItem -Path $baseReleasePath -Recurse -Directory | ForEach-Object {
        if (-not (Test-IsDebugItem $_)) {
            if ($_.Attributes -notmatch "ReparsePoint") {
                $relDir = $_.FullName.Substring($baseReleasePath.Length).TrimStart('\')
                $targetDir = Join-Path -Path $baseBuildPath -ChildPath $relDir
                if (-not (Test-Path -Path $targetDir)) {
                    New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
                }
            }
        }
    }

    # 2. Procesar los elementos (archivos y symlinks)
    Get-ChildItem -Path $baseReleasePath -Recurse | ForEach-Object {

        # Filtrar archivos o directorios de depuración
        if (Test-IsDebugItem $_) {
            return
        }
        
        $relativePath = $_.FullName.Substring($baseReleasePath.Length).TrimStart('\')
        $destPath     = Join-Path -Path $baseBuildPath -ChildPath $relativePath
        $srcPath      = Join-Path -Path $baseSrcPath -ChildPath $relativePath

        # CASO A: Es un Enlace Simbólico (ReparsePoint) -> apunta a Target
        if ($_.Attributes -match "ReparsePoint") {
            if (Test-Path -Path $srcPath) {
                if ($_.PSIsContainer) {
                    # Si el symlink es un directorio, copiamos descartando archivos de debug
                    Get-ChildItem -Path $srcPath -Recurse | ForEach-Object {
                        if (-not (Test-IsDebugItem $_)) {
                            $subRelPath = $_.FullName.Substring($srcPath.Length)
                            $itemDest   = Join-Path -Path $destPath -ChildPath $subRelPath
                            
                            if ($_.PSIsContainer) {
                                if (-not (Test-Path -Path $itemDest)) {
                                    New-Item -ItemType Directory -Path $itemDest -Force | Out-Null
                                }
                            } else {
                                $itemDestDir = Split-Path -Path $itemDest -Parent
                                if (-not (Test-Path -Path $itemDestDir)) {
                                    New-Item -ItemType Directory -Path $itemDestDir -Force | Out-Null
                                }
                                Copy-Item -Path $_.FullName -Destination $itemDest -Force
                            }
                        }
                    }
                } else {
                    # Es un archivo individual
                    Copy-Item -Path $srcPath -Destination $destPath -Force
                }
            } else {
                Write-Warning "No existe el origen real en Target: $srcPath"
            }
        }
        # CASO B: Es un archivo normal de Release
        elseif (-not $_.PSIsContainer) {
            $destDir = Split-Path -Path $destPath -Parent
            if (-not (Test-Path -Path $destDir)) {
                New-Item -ItemType Directory -Path $destDir -Force | Out-Null
            }
            Copy-Item -Path $_.FullName -Destination $destPath -Force
        }
    }
}

function CompressRelease
{
    param (
        [string]$IntermediateFolder,
        [string]$OutputFile = "Release.zip"
    )

    if (-not (Test-Path -Path $IntermediateFolder)) {
        Write-Error "La carpeta origen '$IntermediateFolder' no existe."
        return
    }

    # Si ya existe un archivo ZIP previo, se borra para reescribirlo de forma limpia
    if (Test-Path -Path $OutputFile) {
        Remove-Item -Path $OutputFile -Force
    }

    # Al pasar Join-Path con '*' ($IntermediateFolder\*), Compress-Archive toma
    # únicamente los elementos dentro de la carpeta y los sitúa en la raíz del ZIP
    $sourcePath = Join-Path -Path $IntermediateFolder -ChildPath "*"

    Compress-Archive -Path $sourcePath -DestinationPath $OutputFile -Force
}

CreateRelease -Src $srcfolder -Release $releasefolder -Intermediate $intermediatefolder
CompressRelease -IntermediateFolder $intermediatefolder -OutputFile $zipOutput