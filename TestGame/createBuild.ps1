$game_folder=&"pwd"
$project = "TestGame"

cd .\Editor_Release\

Start-Process "./${project}_Editor_Release.exe" -ArgumentList "--build=mainmenu,venom --boot=mainmenu" -NoNewWindow -Wait

cd $game_folder
