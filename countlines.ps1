cd .\Engine\
(Get-ChildItem -Recurse -Include *.cpp, *.h, *.hpp | Get-Content | Measure-Object -Line).Lines
cd ..

cd .\TestGame\
(Get-ChildItem -Recurse -Include *.cpp, *.h, *.hpp | Get-Content | Measure-Object -Line).Lines
cd ..