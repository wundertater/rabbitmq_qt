cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0016 NEW)

set(CMAKE_CXX_STANDARD 14)

set(TARGET_NAME client)
set(LIB_NAME ClientLib)

find_package(Protobuf REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets Core)

set(CMAKE_AUTOMOC ON)

set(HEADERS
    Client.h
)

set(SOURCES
    Client.cpp
)

add_library(${LIB_NAME} STATIC ${HEADERS} ${SOURCES})
target_link_libraries(${LIB_NAME} PRIVATE RabbitMQClient)
target_link_libraries(${LIB_NAME} PRIVATE messages_protocol protobuf::libprotobuf)

set(EXECUTABLE_HEADERS
    MainWindow.h
    ConfigDialog.h
    MessageReceiverThread.h
)

set(EXECUTABLE_SOURCES
    main.cpp
    MainWindow.cpp
    ConfigDialog.cpp
)

add_executable(${TARGET_NAME} ${EXECUTABLE_HEADERS} ${EXECUTABLE_SOURCES})
target_link_libraries(${TARGET_NAME} PRIVATE ${LIB_NAME})
target_link_libraries(${TARGET_NAME} PRIVATE Logger ConfigManager)
target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Widgets Qt5::Core)
