cmake_minimum_required(VERSION 3.10)

add_library(hmk4_lib model_windgen.cxx scene_pipeline.cxx model_ground.cxx model_cloud.cxx)

add_executable(hmk4_test1 test_model_windgen.cxx )
add_executable(hmk4_test2 test_model_ground.cxx )

foreach(hmk4_targ hmk4_lib hmk4_test1 hmk4_test2)
    target_link_libraries(${hmk4_targ} PUBLIC minimal_framework procedural model)
    # add_test(NAME ${test_targ}_test COMMAND ${test_targ})
endforeach()

foreach(hmk4_targ hmk4_test1 hmk4_test2)
    target_link_libraries(${hmk4_targ} PUBLIC hmk4_lib)    
endforeach()


file(GLOB_RECURSE shader_files_vert "*.vs")
file(GLOB_RECURSE shader_files_frag "*.fs")

install(FILES ${shader_files_vert} ${shader_files_frag} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})