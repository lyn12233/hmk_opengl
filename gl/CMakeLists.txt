cmake_minimum_required(VERSION 3.10)

add_library(gl_wrapped_lib
    buffer_objects.cxx
    glfw_inst.cxx
    shader_program.cxx
    shader.cxx
    texture_objects.cxx
)

target_link_libraries(gl_wrapped_lib PUBLIC
    impl_stb_image_impl
    ${proj_flag}
)

target_include_directories(gl_wrapped_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)