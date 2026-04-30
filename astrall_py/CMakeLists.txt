cmake_minimum_required(VERSION 3.20)

project(astrall_py LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT TARGET astrall_core)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../astrall_core ${CMAKE_CURRENT_BINARY_DIR}/astrall_core)
endif()

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 REQUIRED)

pybind11_add_module(astrall
    src/astrall_bindings.cpp
    src/bind_common.cpp
    src/bind_backend.cpp
    src/bind_control.cpp
    src/bind_planning.cpp
    src/bind_navigation.cpp
    src/bind_behavior.cpp
    src/bind_device.cpp
    src/bind_runtime.cpp
)

target_link_libraries(astrall PRIVATE astrall_core)
