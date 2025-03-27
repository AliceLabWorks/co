cmake_minimum_required(VERSION 3.0)
project(example)

add_executable(base ${CMAKE_CURRENT_SOURCE_DIR}/base_use.cpp)
add_executable(echo_server ${CMAKE_CURRENT_SOURCE_DIR}/echo_server.cpp)
add_executable(async_schedule ${CMAKE_CURRENT_SOURCE_DIR}/async_schedule.cpp)


target_link_libraries(base server_lib)
target_link_libraries(echo_server server_lib)
target_link_libraries(async_schedule server_lib)

