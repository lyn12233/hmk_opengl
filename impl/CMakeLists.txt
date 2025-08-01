cmake_minimum_required(VERSION 3.10)

add_library(impl_stb_image_impl stb_image_impl.cxx)
add_library(impl_stb_image_write_impl stb_image_write_impl.cxx)
add_library(impl_stb_truetype_impl stb_truetype_impl.cxx)
add_library(impl_stb_perlin_impl stb_perlin_impl.cxx)

foreach(impl_lib 
    impl_stb_image_impl 
    impl_stb_image_write_impl
    impl_stb_truetype_impl
    impl_stb_perlin_impl
)
    target_link_libraries(${impl_lib} PUBLIC ${proj_flag})
    target_include_directories(${impl_lib} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()
