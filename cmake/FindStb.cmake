# find stb; export <Stb_INCLUDE_DIR>

include(FetchContent)

FetchContent_Declare(
    Stb
    GIT_REPOSITORY https://gitee.com/ltq12233/stb.git
    GIT_TAG master
)
message(STATUS "fetching stb from https://gitee.com/ltq12233/stb.git")
FetchContent_MakeAvailable(Stb)

message(STATUS "Stb_INCLUDE_DIR: ${stb_SOURCE_DIR}")
if(EXISTS "${stb_SOURCE_DIR}")
    set(Stb_INCLUDE_DIR "${stb_SOURCE_DIR}")
    set(Stb_FOUND true)
else()
    message(FATAL_ERROR "find_package(Stb) failed")
endif()