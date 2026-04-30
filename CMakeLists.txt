cmake_minimum_required(VERSION 3.20)

project(astrall_workspace LANGUAGES CXX)

option(ASTRALL_BUILD_PYTHON "Build the astrall pybind11 module" ON)

enable_testing()

add_subdirectory(astrall_core)

if(ASTRALL_BUILD_PYTHON)
    add_subdirectory(astrall_py)
endif()
