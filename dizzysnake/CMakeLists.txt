cmake_minimum_required(VERSION 3.14)
include(${CMAKE_CURRENT_SOURCE_DIR}/../CMakeLists.txt)

set(NEEDED_LIBRARIES
)

set(NEEDED_MODES
    snake
)

# Game Name Customization
set(PLAYDATE_GAME_NAME $ENV{PD_PROJECT_NAME})
set(PLAYDATE_GAME_DEVICE ${PLAYDATE_GAME_NAME}_DEVICE)

set(NEEDED_LIBRARIES_H)
set(NEEDED_LIBRARIES_C)

foreach(Lib IN ITEMS ${NEEDED_LIBRARIES})
    set(LibH "../library/${Lib}.h")
    set(LibC "../library/${Lib}.c")
    list(APPEND NEEDED_LIBRARIES_H ${LibH})
    list(APPEND NEEDED_LIBRARIES_C ${LibC}) 
endforeach()

foreach(Mode IN ITEMS ${NEEDED_MODES})
    set(ModeH "../mode/${Mode}.h")
    set(ModeC "../mode/${Mode}.c")
    list(APPEND NEEDED_MODES_H ${ModeH})
    list(APPEND NEEDED_MODES_C ${ModeC}) 
endforeach()

file(GLOB SOURCES_H src/*.h ${NEEDED_LIBRARIES_H} ${NEEDED_MODES_H} ../core/*.h)

# TODO: add test.h
file(GLOB SOURCES src/*.c ${NEEDED_LIBRARIES_C} ${NEEDED_MODES_C} ../core/*.c ${SOURCES_H})

project(${PLAYDATE_GAME_NAME} C ASM)

if (TOOLCHAIN STREQUAL "armgcc")
	add_executable(${PLAYDATE_GAME_DEVICE} ${SDK}/C_API/buildsupport/setup.c ${SOURCES})
else()
	add_library(${PLAYDATE_GAME_NAME} SHARED ${SOURCES})
endif()

include(${SDK}/C_API/buildsupport/playdate_game.cmake)
