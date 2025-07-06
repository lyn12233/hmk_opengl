cmake_minimum_required(VERSION 3.10)

add_library(procedural 
    volumetric_cloud.cxx
)

target_link_libraries(procedural PUBLIC
    gl_wrapped_lib
    impl_stb_perlin_impl
    minimal_framework
    ${proj_flag}
)

target_include_directories(procedural PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)