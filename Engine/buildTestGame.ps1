$engine_folder=&"pwd"

$msbuild = &"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe
$configurations = @("Debug", "Editor_Debug", "Development", "Editor_Release", "Editor_Development", "Editor_SoftDebug", "Release")
$project = "TestGame"

function BuildProject {
	param(
        [string]$Configuration
    )
	
	& $msbuild ./Engine.sln /p:Configuration=$Configuration -p:Platform=x64 /t:Rebuild
}

foreach ($conf in $configurations) {
	BuildProject $conf
}

cd ..\$project\Editor_Release\

Start-Process "./${project}_Editor_Release.exe" -ArgumentList "--generate-dds" -NoNewWindow -Wait

cd $engine_folder
