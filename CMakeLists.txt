cmake_minimum_required(VERSION 3.26)
project(NanoContainer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(third_party)
include_directories(include)

file(GLOB_RECURSE INCLUDE_DIRS LIST_DIRECTORIES true include/*)
foreach(DIR ${INCLUDE_DIRS})
    if(IS_DIRECTORY ${DIR})
        include_directories(${DIR})
    endif()
endforeach()

aux_source_directory(src/cmd CMD_SOURCES)
aux_source_directory(src/container CONTAINER_SOURCES)
aux_source_directory(src/deamon DEAMON_SOURCES)
aux_source_directory(src/image IMAGE_SOURCES)
aux_source_directory(src/proto PROTO_SOURCES)
aux_source_directory(src/rootfs ROOTFS_SOURCES)
aux_source_directory(src/utils UTILS_SOURCES)
aux_source_directory(src/runtime RUNTIME_SOURCES)
aux_source_directory(src/network NETWORK_SOURCES)
aux_source_directory(src/daemon DEAMON_SOURCES)
aux_source_directory(src/protocol PROTOCOL_SOURCES)
aux_source_directory(src/builder BUILDER_SOURCES)

#list 
list(APPEND SOURCES
    ${CMD_SOURCES} 
    ${CONTAINER_SOURCES}
    ${DEAMON_SOURCES}
    ${IMAGE_SOURCES}
    ${PROTO_SOURCES} 
    ${ROOTFS_SOURCES} 
    ${UTILS_SOURCES}
    ${RUNTIME_SOURCES}
    ${NETWORK_SOURCES}
    ${DEAMON_SOURCES}
    ${PROTOCOL_SOURCES}
    ${BUILDER_SOURCES}
    )

add_executable(NanoContainer ${SOURCES} main.cpp)

#configure_file(Dockerfile ${CMAKE_BINARY_DIR}/Dockerfile COPYONLY)