cmake_minimum_required(VERSION 3.10)

add_executable(hello_triangle hello_triangle.cxx)
add_executable(move_triangle move_triangle.cxx)
add_executable(use_text use_text.cxx)
add_executable(test_lighting test_lighting.cxx mycube.cxx)

foreach(test_targ hello_triangle move_triangle use_text test_lighting)
    target_link_libraries(${test_targ} PUBLIC minimal_framework)
    add_test(NAME ${test_targ}_test COMMAND ${test_targ})
endforeach()


file(GLOB_RECURSE shader_files "shader*")
file(GLOB_RECURSE tex_files "tex*")
install(FILES ${shader_files} ${tex_files} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

