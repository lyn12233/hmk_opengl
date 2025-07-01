# find glm; export target glm::glm

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://gitee.com/ltq12233/glm.git
    GIT_TAG master
)

message(STATUS "fetching glm from https://gitee.com/ltq12233/glm.git")
FetchContent_MakeAvailable(glm)

if(TARGET glm::glm)
    message("found target: glm::glm")
    set(glm_FOUND true)
else()
    message(FATAL_ERROR "find_package(glm) failed")
endif()