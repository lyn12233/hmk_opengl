cmake_minimum_required(VERSION 3.10)

add_executable(defer_render_test defer_render_test.cxx)

foreach(test_targ defer_render_test)
    target_link_libraries(${test_targ} PUBLIC minimal_framework procedural model)
    add_test(NAME ${test_targ}_test COMMAND ${test_targ})
endforeach()


file(GLOB_RECURSE shader_files "shader*")
file(GLOB_RECURSE tex_files "tex*")
install(FILES ${shader_files} ${tex_files} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})