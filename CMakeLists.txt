# CMakeLists files in this project can
cmake_minimum_required (VERSION 2.8.11)

project(tcp_server_client_wrapper)

find_package(Threads REQUIRED)

add_library (
    tcp_server_client_wrapper
    app/src/tcp_server_client_wrapper.cpp
    )

target_include_directories ( tcp_server_client_wrapper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/app/include )

target_link_libraries( tcp_server_client_wrapper "${CMAKE_THREAD_LIBS_INIT}" )

add_executable(
    tcp_server_client_wrapper_test
    test/test.cpp
)

target_link_libraries( tcp_server_client_wrapper_test tcp_server_client_wrapper )
