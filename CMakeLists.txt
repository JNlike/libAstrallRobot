cmake_minimum_required(VERSION 3.20)

project(astrall_robot_runtime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(pybind11 REQUIRED)
find_package(yaml-cpp REQUIRED)

add_library(astrall_core
    src/backend/sim_backend.cpp
    src/backend/real_backend.cpp
    src/control/controller.cpp
    src/planning/straight_line_planner.cpp
    src/navigation/navigator.cpp
    src/behavior/state_machine.cpp
    src/device/dummy_camera.cpp
    src/device/dummy_radar.cpp
    src/runtime.cpp
)

target_include_directories(astrall_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(astrall_core
    PUBLIC
        yaml-cpp
)

pybind11_add_module(astrall
    bindings/python/astrall_bindings.cpp
    bindings/python/bind_common.cpp
    bindings/python/bind_backend.cpp
    bindings/python/bind_control.cpp
    bindings/python/bind_planning.cpp
    bindings/python/bind_navigation.cpp
    bindings/python/bind_behavior.cpp
    bindings/python/bind_device.cpp
    bindings/python/bind_runtime.cpp
)

target_link_libraries(astrall PRIVATE astrall_core)

add_executable(astrall_cpp_demo examples/cpp_demo.cpp)
target_link_libraries(astrall_cpp_demo PRIVATE astrall_core)
