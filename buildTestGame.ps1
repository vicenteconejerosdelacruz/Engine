$engine_folder=&"pwd"

$configurations = @("Debug", "Editor_Debug", "Development", "Editor_Release", "Editor_Development", "Release")
$project = "TestGame"

function BuildProject {
	param(
        [string]$Configuration
    )
	
	cmake --build build --config $Configuration --parallel --clean-first
}

cmd /c mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cd ..

foreach ($conf in $configurations) {
	BuildProject $conf
}

cd $project\Editor_Release\

Start-Process "./${project}_Editor_Release.exe" -ArgumentList "--generate-dds" -NoNewWindow -Wait

cd $engine_folder
