# find glfw(3); export target: glfw

include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://gitee.com/ltq12233/glfw.git
    GIT_TAG 3.4
)

message(STATUS "fectching glfw3")
FetchContent_MakeAvailable(glfw)

if(TARGET glfw)
    message(STATUS "found target: glfw")
    set(glfw3_FOUND true)
else()
    message(FATAL_ERROR "find_package(glfw3) failed")
endif()