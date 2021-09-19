cmake_minimum_required(VERSION 3.20.2)
project(hyperionapplication VERSION 0.1.0)


include(CTest)
enable_testing()

include_directories(
    ../hyperion-library/include)

link_directories(../hyperion-library/build/MinSizeRel)

add_executable(hyperionconsole main.cpp)

target_link_libraries(hyperionconsole hyperionlibrary.lib)

set_property(TARGET hyperionconsole PROPERTY CXX_STANDARD 17)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
