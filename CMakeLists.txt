cmake_minimum_required(VERSION 3.0)

project(my_server VERSION 1.0)

SET(CMAKE_COLOR_MAKEFILE    ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-Wall -g -fPIC -std=c++17 -D_GLIBCXX_USE_CXX11_ABI=0 -DASIO_STANDALONE -Wno-deprecated -Wno-narrowing -fdiagnostics-color=always -fno-var-tracking-assignments -Wno-misleading-indentation -Wno-class-memaccess")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -g")
add_compile_options(-gdwarf-4)
# 启用汇编支持
enable_language(ASM)

# 指定源文件
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.S
)

include_directories(
    ${CMAKE_SOURCE_DIR}/3party
    ${CMAKE_SOURCE_DIR}/include
)

link_directories(${CMAKE_SOURCE_DIR}/3party/boost/context/lib)

add_library(server_lib ${SOURCES})

target_link_libraries(server_lib boost_context pthread)

add_subdirectory(example)

