cmake_minimum_required(VERSION 3.10)
project(VPCIE C)

set(CMAKE_C_STANDARD 99)

add_library(vpcie net/vpcienet.h net/vpcienet.c)

IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
    SET_TARGET_PROPERTIES(vpcie PROPERTIES COMPILE_FLAGS "-fPIC")
ENDIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

find_package(Threads REQUIRED)
add_library(vpi SHARED vpi/vpi.c)
target_include_directories(vpi PUBLIC /opt/remote/rsstudent/tools/modelsim/default/modeltech/include)

target_link_libraries(vpi vpcie ${CMAKE_THREAD_LIBS_INIT})
