# find spdlog; export target: spdlog::spdlog

include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://gitee.com/ltq12233/spdlog.git
    GIT_TAG v1.15.0
)
message(STATUS "fetching spdlog")
FetchContent_MakeAvailable(spdlog)

if(TARGET spdlog::spdlog)
    message(STATUS "found target: spdlog::spdlog")
    set(spdlog_FOUND true)
else()
    message(FATAL_ERROR "find_package(spdlog) failed")
endif()