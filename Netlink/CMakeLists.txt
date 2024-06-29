project(Netlink_Sockets)

# Find the path to make binary and strip the newline character
# NOTE We cannot use CMAKE_MAKE_PROGRAM instead because it is set based on cmake generator used which could also be Ninja
execute_process(COMMAND which make 
  OUTPUT_VARIABLE MAKE_PROGRAM
)
string(STRIP ${MAKE_PROGRAM} MAKE_PROGRAM)

set(NETLINK_KERN_MODULE kernel_client)
set(NETLINK_MOD_FILE ${NETLINK_KERN_MODULE}.ko)
set(NETLINK_MOD_SRC_FILES ${NETLINK_KERN_MODULE}.c)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME})
file(GLOB SRC_FILES user*.c user*.cpp)

foreach(source_file ${SRC_FILES})
    get_filename_component(BINARY_NAME ${source_file} NAME_WE)
    add_executable(${BINARY_NAME} ${source_file})
endforeach(source_file ${SRC_FILES})

add_custom_command(OUTPUT ${NETLINK_MOD_FILE}
  COMMAND ${MAKE_PROGRAM} all
  COMMAND ${CMAKE_COMMAND} -E copy ${NETLINK_MOD_FILE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${MAKE_PROGRAM} clean
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${NETLINK_MOD_SRC_FILES}
  COMMENT "Building netlink kernel module"
  VERBATIM
)

add_custom_target(${NETLINK_KERN_MODULE} ALL DEPENDS ${NETLINK_MOD_FILE})
# Clean netlink module output from the output directory on running a clean cmake target
set_target_properties(${NETLINK_KERN_MODULE} 
  PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NETLINK_MOD_FILE}
)