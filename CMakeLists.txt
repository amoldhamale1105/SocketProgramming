## Top level cmake to build all projects ##
cmake_minimum_required(VERSION 3.0.0)

add_subdirectory(TCP)
add_subdirectory(UDP)
add_subdirectory(Unix)