cmake_minimum_required(VERSION 3.20.0)


set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(BOARD h723vg)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/prj.conf")
  list(APPEND CONF_FILE "prj.conf")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky)

target_sources(app PRIVATE src/main.c)
