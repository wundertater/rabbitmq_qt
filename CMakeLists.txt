cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0016 NEW)

project(rabbitmq-qt VERSION 1.0.0 DESCRIPTION "Использование брокера сообщений")

include_directories(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(src/RabbitMQClient)
add_subdirectory(src/Logger)
add_subdirectory(src/ConfigManager)
add_subdirectory(src/protocol)
add_subdirectory(src/server)
add_subdirectory(src/client)

enable_testing()
add_subdirectory(test/server)
add_subdirectory(test/client)
add_subdirectory(integration-test)
