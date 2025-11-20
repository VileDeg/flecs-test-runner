cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME "TestRunner")

# Set the project name and version
project(${PROJECT_NAME} VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)
