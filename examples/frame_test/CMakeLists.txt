cmake_minimum_required(VERSION 3.10)

add_executable(display_arguments display_arguments.cxx)

foreach(test_targ display_arguments)
    target_link_libraries(${test_targ} PUBLIC minimal_framework procedural)
    add_test(NAME ${test_targ}_test COMMAND ${test_targ})
endforeach()