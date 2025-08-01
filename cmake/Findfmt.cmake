# find assimp, export fmt::fmt

include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://gitee.com/ltq12233/fmt.git
    GIT_TAG master
)
message(STATUS "fetching fmt")
FetchContent_MakeAvailable(fmt)

message(STATUS "Stb_INCLUDE_DIR: ${stb_SOURCE_DIR}")
if(EXISTS "${stb_SOURCE_DIR}")
    set(Stb_INCLUDE_DIR "${stb_SOURCE_DIR}")
    set(Stb_FOUND true)
else()
    message(FATAL_ERROR "find_package(fmt) failed")
endif()