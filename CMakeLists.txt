cmake_minimum_required(VERSION 3.10)

set(proj_name gl_wrapped_utilities)
set(proj_flag ${proj_name}_flags)

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

enable_testing()

project(${proj_name})


# find package

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(glm REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(Stb REQUIRED)
find_package(assimp REQUIRED)
# find_package(fmt REQUIRED)


# generic compile options/flags

add_library(${proj_flag} INTERFACE)

target_link_libraries(${proj_flag} INTERFACE 
    spdlog::spdlog
    glfw
    glad::glad
    glm::glm
    assimp::assimp
)
target_include_directories(${proj_flag} INTERFACE 
    ${Stb_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

target_compile_features(${proj_flag} INTERFACE cxx_std_17)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # target_compile_options(${proj_flag} INTERFACE -g -gdwarf-4 -O0)
    # target_link_options(${proj_flag} INTERFACE -g -gdwarf-4)
    # target_compile_options(${proj_flag} INTERFACE -fsanitize=address -fno-omit-frame-pointer)
    # target_link_options(${proj_flag} INTERFACE -fsanitize=address)

    # message("setting _ITERATOR_DEBUG_LEVEL=0")
    # target_compile_definitions(${proj_flag} INTERFACE _ITERATOR_DEBUG_LEVEL=0)
    # set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    
endif()


# add sub-directories

add_subdirectory(impl)
add_subdirectory(gl)
add_subdirectory(framework)
add_subdirectory(procedural)
add_subdirectory(model)

option(UTILS_BUILD_EXAMPLES "Build example subdirectory" ON)

if(UTILS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()


# simple test
add_executable(simple_test test.cxx)
target_link_libraries(simple_test ${proj_flag})