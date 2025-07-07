cmake_minimum_required(VERSION 3.10)

set(proj_name gl_wrapped_utilities)
set(proj_flag ${proj_name}_flags)

enable_testing()

project(${proj_name})

# add dependdencies using fetch_content

# include(FetchContent)

# # FetchContent declare
# FetchContent_Declare(
#     glfw
#     GIT_REPOSITORY https://gitee.com/scu319hy/glfw.git
#     GIT_TAG 3.3.8
# )
# FetchContent_Declare(
#     stb
#     GIT_REPOSITORY https://gitee.com/mirrors_KeyWeeUsr/stb.git
#     GIT_TAG master
# )
# FetchContent_Declare(
#     spdlog
#     GIT_REPOSITORY https://gitee.com/ltq12233/spdlog.git
#     GIT_TAG v1.15.0
# )
# FetchContent_Declare(
#     glad
#     GIT_REPOSITORY https://gitee.com/ltq12233/glad.git
#     GIT_TAG v2.0.8
# )

# # disable 3rd party test builds
# foreach(suffix GLFW SPDLOG GLAD)
#     foreach(prefix TESTS TEST EXAMPLES EXAMPLE DOCS DOC)
#         set(${suffix}_BUILD_${prefix} OFF CACHE BOOL "" FORCE)
#     endforeach()
# endforeach()


# # FetchContent make available
# message(STATUS "fetching glfw from https://gitee.com/scu319hy/glfw.git (mirror)")
# FetchContent_MakeAvailable(glfw)
# message(STATUS "fetching stb from https://gitee.com/mirrors_KeyWeeUsr/stb.git (mirror)")
# FetchContent_MakeAvailable(stb)
# message(STATUS "stb source dir: ${stb_SOURCE_DIR}")
# message(STATUS "fetching spdlog from https://gitee.com/ltq12233/spdlog.git (mirror)")
# FetchContent_MakeAvailable(spdlog)
# message(STATUS "fetching glad from https://gitee.com/ltq12233/glad.git (mirror)")
# FetchContent_MakeAvailable(glad)
# message(STATUS "glad source dir: ${glad_SOURCE_DIR}")
# include(${glad_SOURCE_DIR}/cmake/gladConfig.cmake)
# glad_add_library(glad_gl_core_45 STATIC API gl:core=4.5)

# generic compile options/flags

# find package
# list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(glm REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(Stb REQUIRED)

add_library(${proj_flag} INTERFACE)

target_link_libraries(${proj_flag} INTERFACE 
    spdlog::spdlog
    glfw
    glad::glad
    glm::glm
)
target_include_directories(${proj_flag} INTERFACE 
    ${Stb_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

target_compile_features(${proj_flag} INTERFACE cxx_std_17)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${proj_flag} INTERFACE -g -gdwarf-4 -O0)
    target_link_options(${proj_flag} INTERFACE -g -gdwarf-4)
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "need to disable lld-link warning")
        target_link_options(${proj_flag} INTERFACE -Wl,/IGNORE:ALL)
    endif()
endif()


# add sub-directories

add_subdirectory(impl)
add_subdirectory(gl)
add_subdirectory(framework)
add_subdirectory(procedural)

option(UTILS_BUILD_EXAMPLES "Build example subdirectory" ON)

if(UTILS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()


# simple test
add_executable(simple_test test.cxx)
target_link_libraries(simple_test ${proj_flag})