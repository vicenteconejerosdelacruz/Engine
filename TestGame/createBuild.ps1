$game_folder = Get-Location
$project = "TestGame"
$build_folder = Join-Path -Path $game_folder -ChildPath "Build"

# Generar el timestamp (ej. 20260924_150110)
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$zipOutput = "${project}_${timestamp}.zip"

# Executar el proceso de Build
Set-Location -Path ".\Editor_Release\"
Start-Process "./${project}_Editor_Release.exe" -ArgumentList "--build=boot,mainmenu,venom --boot=boot" -NoNewWindow -Wait
Set-Location -Path $game_folder

# Comprimir y limpiar si la carpeta Build existe
if (Test-Path -Path $build_folder) {
    Write-Host "Comprimiendo contenido de Build en $zipOutput..." -ForegroundColor Cyan

    # El uso de \* asegura que el contenido de Build quede en la raíz del ZIP
    $sourcePath = Join-Path -Path $build_folder -ChildPath "*"
    $zipFullPath = Join-Path -Path $game_folder -ChildPath $zipOutput

    if (Test-Path -Path $zipFullPath) {
        Remove-Item -Path $zipFullPath -Force
    }

    Compress-Archive -Path $sourcePath -DestinationPath $zipFullPath -Force
    Write-Host "Archivo $zipOutput creado con éxito." -ForegroundColor Green

    # Eliminar la carpeta Build
    Write-Host "Eliminando carpeta temporal $build_folder..." -ForegroundColor Yellow
    Remove-Item -Path $build_folder -Recurse -Force
} else {
    Write-Warning "No se encontró la carpeta Build en $build_folder. No se generó el ZIP."
}