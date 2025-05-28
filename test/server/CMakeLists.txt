cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0016 NEW)

set(TEST_PROJECT_NAME ServerTest)
set(CMAKE_CXX_STANDARD 14)

find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)

set(SOURCES
    Test_Server.cpp
    ${PROJECT_SOURCE_DIR}/test/common/mocks.h
)

add_executable(${TEST_PROJECT_NAME} ${SOURCES})

target_include_directories(${TEST_PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src/server ${GTEST_INCLUDE_DIRS})
target_include_directories(${TEST_PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/test/common)

target_link_libraries(${TEST_PROJECT_NAME} PRIVATE ServerLib Logger)
target_link_libraries(${TEST_PROJECT_NAME} PRIVATE GTest::gtest_main GTest::gmock)

include(GoogleTest)
gtest_discover_tests(${TEST_PROJECT_NAME})
