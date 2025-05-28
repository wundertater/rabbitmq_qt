cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0016 NEW)

set(CMAKE_CXX_STANDARD 14)

set(TARGET_NAME RabbitMQClient)

find_package(Qt5 REQUIRED COMPONENTS Core)

find_library(LIBRABBITMQ_LIBRARY
    NAMES rabbitmq
    PATHS ${LIBRABBITMQ_LIBRARY_DIRS}
  )

set(HEADERS
    IRabbitmqConnection.h
    RabbitmqConnection.h
    rabbitmqEntities.h
    validation.h
)

set(SOURCES
    RabbitmqConnection.cpp
    rabbitmqEntities.cpp
    validation.cpp
)

add_library(${TARGET_NAME} STATIC ${HEADERS} ${SOURCES})

target_include_directories(${TARGET_NAME} PRIVATE ${LIBRABBITMQ_STATIC_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${LIBRABBITMQ_LIBRARY})
target_link_libraries(${TARGET_NAME} PUBLIC Qt5::Core)
