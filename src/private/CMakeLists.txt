project(avm C ASM)

include_directories(
    ${AVM_ROOT_DIRECTORY}/src/inc
    ${VALGRIND_INCLUDE_DIR})

file(GLOB_RECURSE PUB_HEADERS ${AVM_ROOT_DIRECTORY}/src/inc/*.h)
file(GLOB_RECURSE HEADERS 	  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES 	  ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE ASM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.S)

add_library(avm SHARED ${PUB_HEADERS} ${HEADERS} ${SOURCES} ${ASM_SOURCES})

add_sanitizers(avm)

add_definitions(-DANY_EXPORT -DANY_SHARED)

if(SANITIZE_ADDRESS)
    add_definitions(-DANY_USE_ASAN)
endif()

if(UNIX AND NOT APPLE)
	target_link_libraries(avm rt)
endif()