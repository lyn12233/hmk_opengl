# find glad; export target glad::glad

get_filename_component(GLAD_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

add_library(glad STATIC  ${GLAD_CMAKE_DIR}/glad/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_CMAKE_DIR}/glad/include/)

add_library(glad::glad ALIAS glad)

set(glad_FOUND true)