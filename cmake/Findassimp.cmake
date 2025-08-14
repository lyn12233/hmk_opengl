# find assimp, export assimp::assimp

include(FetchContent)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)        
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLB_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://gitee.com/ltq12233/assimp.git
    GIT_TAG master
)
message(STATUS "fetching assimp")
FetchContent_MakeAvailable(assimp)
target_compile_definitions(assimp PRIVATE _CRT_SECURE_NO_WARNINGS)

message(STATUS "Stb_INCLUDE_DIR: ${stb_SOURCE_DIR}")
if(EXISTS "${stb_SOURCE_DIR}")
    set(Stb_INCLUDE_DIR "${stb_SOURCE_DIR}")
    set(Stb_FOUND true)
else()
    message(FATAL_ERROR "find_package(assimp) failed")
endif()