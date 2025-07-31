cmake_minimum_required(VERSION 3.10)

add_executable(display_arguments display_arguments.cxx)
add_executable(load_model load_model.cxx)
add_executable(paste_tex paste_tex.cxx)
add_executable(test_button test_button.cxx)

foreach(test_targ display_arguments load_model paste_tex test_button)
    target_link_libraries(${test_targ} PUBLIC minimal_framework procedural model)
    add_test(NAME ${test_targ}_test COMMAND ${test_targ})
endforeach()

file(GLOB_RECURSE shader_files "shader*")
file(GLOB_RECURSE tex_files "tex*")
file(GLOB_RECURSE model_files "cabin*")

install(FILES ${shader_files} ${tex_files} ${model_files} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})