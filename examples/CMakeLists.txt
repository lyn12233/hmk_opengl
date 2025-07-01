cmake_minimum_required(VERSION 3.10)

add_executable(use_text use_text.cxx)

foreach(test_targ use_text)
    target_link_libraries(${test_targ} PUBLIC minimal_framework)
endforeach()
