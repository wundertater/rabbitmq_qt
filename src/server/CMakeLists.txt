cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0016 NEW)

set(CMAKE_CXX_STANDARD 14)

set(TARGET_NAME server)
set(LIB_NAME ServerLib)

find_package(Protobuf REQUIRED)

set(HEADERS
    Server.h
)

set(SOURCES
    Server.cpp
)

add_library(${LIB_NAME} STATIC ${HEADERS} ${SOURCES})
target_link_libraries(${LIB_NAME} PRIVATE RabbitMQClient)
target_link_libraries(${LIB_NAME} PRIVATE messages_protocol protobuf::libprotobuf)


set(EXECUTABLE_SOURCES
    main.cpp
)

add_executable(${TARGET_NAME} ${EXECUTABLE_SOURCES})
target_link_libraries(${TARGET_NAME} PRIVATE ${LIB_NAME})
target_link_libraries(${TARGET_NAME} PRIVATE Logger ConfigManager)
