cmake_minimum_required(VERSION 3.20.0)

set(BOARD esp32)

find_package(Zephyr)

project(vortice_v2)

target_sources(app PRIVATE src/main.c)
