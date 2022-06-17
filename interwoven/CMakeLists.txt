cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../CMakeLists.txt)

# Game Name Customization
set(PLAYDATE_GAME_NAME $ENV{PD_PROJECT_NAME})
set(PLAYDATE_GAME_DEVICE ${PLAYDATE_GAME_NAME}_DEVICE)

set(NEEDED_LIBRARIES
)

set(NEEDED_LIBRARIES_H)
set(NEEDED_LIBRARIES_C)

foreach(Lib IN ITEMS ${NEEDED_LIBRARIES})
    set(LibH "../library/${Lib}.h")
    set(LibC "../library/${Lib}.c")
    list(APPEND NEEDED_LIBRARIES_H ${LibH})
    list(APPEND NEEDED_LIBRARIES_C ${LibC}) 
endforeach()

file(GLOB SOURCES_H src/*.h ${NEEDED_LIBRARIES_H})

# TODO: add test.h
file(GLOB SOURCES src/*.c ${SOURCES_H} ${NEEDED_LIBRARIES_C})

project(${PLAYDATE_GAME_NAME} C ASM)

if (TOOLCHAIN STREQUAL "armgcc")
	add_executable(${PLAYDATE_GAME_DEVICE} ${SDK}/C_API/buildsupport/setup.c ${SOURCES})
else()
	add_library(${PLAYDATE_GAME_NAME} SHARED ${SOURCES})
endif()

include(${SDK}/C_API/buildsupport/playdate_game.cmake)