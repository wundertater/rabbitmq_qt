cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0016 NEW)

set(CMAKE_CXX_STANDARD 14)

set(TARGET_NAME Logger)

find_package(Qt5 REQUIRED COMPONENTS Core)

set(HEADERS
    Logger.h
)

set(SOURCES
    Logger.cpp
)

add_library(${TARGET_NAME} STATIC ${HEADERS} ${SOURCES})

target_link_libraries(${TARGET_NAME} Qt5::Core)
