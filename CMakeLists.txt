cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(vortice_v2)

file(GLOB APP_SOURCES CONFIGURE_DEPENDS 
    "**/*.c"
    "**/*.h")

target_sources(app PRIVATE ${APP_SOURCES})
