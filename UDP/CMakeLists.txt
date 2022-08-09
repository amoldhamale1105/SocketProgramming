cmake_minimum_required(VERSION 3.0.0)

project(UDP_Sockets VERSION 1.0.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME})
file(GLOB SRC_FILES *.c *.cpp)

foreach(source_file ${SRC_FILES})
    get_filename_component(BINARY_NAME ${source_file} NAME_WE)
    add_executable(${BINARY_NAME} ${source_file})
endforeach(source_file ${SRC_FILES})