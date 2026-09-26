#run like this
#mkdir build
#cd build
#cmake -G "Visual Studio 17 2022" -A x64 ..

cmake_minimum_required(VERSION 3.20)

# Define explícitamente las configuraciones personalizadas en la caché
set(CMAKE_CONFIGURATION_TYPES 
    "Debug" "Development" "Release" 
    "Editor_Debug" "Editor_Development" "Editor_Release" 
    CACHE STRING "Custom build configurations" FORCE
)

project(Engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Incluye subcarpetas
add_subdirectory(Engine)
add_subdirectory(TestGame)