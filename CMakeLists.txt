cmake_minimum_required(VERSION 3.17)
project(learnopengl_voxel_experiment)

set(LibraryRoot "G:/Programming/C++ Libraries/")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_CXX_FLAGS  "-std=c++2a -fopenmp -O3 -msse2 -msse4.1 -mavx2 -mavx512f -fno-asynchronous-unwind-tables")
set(CMAKE_CXX_FLAGS  "-std=c++2a -fopenmp -O3 -static -static-libgcc -static-libstdc++")
include_directories(src)

include_directories("${LibraryRoot}single_header")
include_directories("${LibraryRoot}entt-master/single_include")
include_directories("${LibraryRoot}glfw-3.3.2.bin.WIN64/glfw-3.3.2.bin.WIN64/include")
include_directories("${LibraryRoot}glad_gl4.3/include")
include_directories("${LibraryRoot}glm")
add_library(glad "${LibraryRoot}glad_gl4.3/include/glad/glad.h" "${LibraryRoot}glad_gl4.3/src/glad.c")


add_executable(${PROJECT_NAME} src/main.cpp src/graphics/Shader.cpp src/graphics/Shader.h src/ecs/Components.h src/ecs/systems/terrain/ChunkManagement.cpp
        src/ecs/systems/terrain/ChunkManagement.h src/ecs/systems/Physics.cpp src/ecs/systems/Physics.h
        src/ecs/systems/PlayerControl.cpp src/ecs/systems/PlayerControl.h src/other/Constants.h src/ecs/Components.cpp
        src/other/DataTypes.cpp src/other/DataTypes.h src/ecs/World.cpp src/ecs/World.h
        src/external/fastnoise/FastNoise.h src/external/fastnoise/FastNoise.cpp src/external/ThreadPool.h src/other/CustomAlgorithms.cpp src/other/CustomAlgorithms.h src/ecs/systems/AI.cpp src/ecs/systems/AI.h src/ecs/systems/Graphics.cpp src/ecs/systems/Graphics.h src/graphics/TextureManager.cpp src/graphics/TextureManager.h src/ecs/systems/terrain/TerrainFunctions.h src/ecs/systems/terrain/TerrainFunctions.cpp src/ecs/systems/terrain/NoisePosTerrain.h src/ecs/systems/terrain/CoordCrossFade.h src/ecs/systems/terrain/CoordShift.h src/ecs/systems/terrain/TerrainCrossFade.h src/ecs/systems/terrain/MainTerrainFunction.h src/other/Profiler.cpp src/other/Profiler.h)
target_link_libraries(${PROJECT_NAME} "${LibraryRoot}glfw-3.3.2.bin.WIN64/glfw-3.3.2.bin.WIN64/lib-mingw-w64/libglfw3.a")
target_link_libraries(${PROJECT_NAME} glad)