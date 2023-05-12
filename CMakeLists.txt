## Top level cmake to build all projects ##
cmake_minimum_required(VERSION 3.0.0)

project(SocketProgramming)

add_subdirectory(TCP)
add_subdirectory(UDP)
add_subdirectory(Unix)
#add_subdirectory(CAN)
add_subdirectory(Netlink)