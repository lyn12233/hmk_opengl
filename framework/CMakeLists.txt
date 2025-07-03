cmake_minimum_required(VERSION 3.10)

add_library(minimal_framework
    drawable_frame.cxx
    sizer.cxx
    textedit.cxx
    widget.cxx
    window.cxx
    world_view.cxx
)

target_link_libraries(minimal_framework PUBLIC
    gl_wrapped_lib
    impl_stb_truetype_impl
    ${proj_flag}
)

target_include_directories(minimal_framework PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)