cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0016 NEW)

set(TEST_PROJECT_NAME IntegrationTest)
set(CMAKE_CXX_STANDARD 14)

find_package(GTest REQUIRED)

set(SOURCES
    Test_Integration.cpp
)

add_executable(${TEST_PROJECT_NAME} ${SOURCES})

target_include_directories(${TEST_PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src/server
    ${PROJECT_SOURCE_DIR}/src/client
    ${GTEST_INCLUDE_DIRS})

target_link_libraries(${TEST_PROJECT_NAME} PRIVATE ServerLib ClientLib Logger)
target_link_libraries(${TEST_PROJECT_NAME} PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${TEST_PROJECT_NAME})
