# find glfw(3); export target: glfw

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://gitee.com/ltq12233/glfw.git
    GIT_TAG 3.4
)

message(STATUS "fectching glfw(3) from https://gitee.com/ltq12233/glfw.git")
FetchContent_MakeAvailable(glfw)

if(TARGET glfw)
    message(STATUS "found target: glfw")
    set(glfw3_FOUND true)
else()
    message(FATAL_ERROR "find_package(glfw3) failed")
endif()