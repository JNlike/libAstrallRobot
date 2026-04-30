cmake_minimum_required(VERSION 3.20)

project(astrall_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ASTRALL_BUILD_DEMOS "Build core demo executables" ON)
option(ASTRALL_BUILD_TESTS "Build core CTest tests" ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(ASTRALL_ENABLE_SDK_DEFAULT ON)
else()
    set(ASTRALL_ENABLE_SDK_DEFAULT OFF)
endif()

option(ASTRALL_ENABLE_SDK
    "Build Astrall SDK integration. Defaults ON on Linux and OFF elsewhere."
    ${ASTRALL_ENABLE_SDK_DEFAULT}
)

find_package(yaml-cpp REQUIRED)

if(ASTRALL_ENABLE_SDK)
    add_subdirectory(lib/astrall_sdk)
endif()

if(TARGET yaml-cpp::yaml-cpp)
    set(ASTRALL_YAML_TARGET yaml-cpp::yaml-cpp)
else()
    set(ASTRALL_YAML_TARGET yaml-cpp)
endif()

set(ASTRALL_CORE_SOURCES
    src/backend/sim_backend.cpp
    src/backend/real_backend.cpp
    src/control/controller.cpp
    src/control/twist_command_mapper.cpp
    src/planning/straight_line_planner.cpp
    src/navigation/navigator.cpp
    src/behavior/state_machine.cpp
    src/device/dummy_camera.cpp
    src/device/dummy_radar.cpp
    src/runtime.cpp
)

set(ASTRALL_CORE_LIBS
    ${ASTRALL_YAML_TARGET}
)

if(ASTRALL_ENABLE_SDK)
    list(APPEND ASTRALL_CORE_SOURCES
        src/sdk/astrall_sdk_wrapper.cpp
    )
    list(APPEND ASTRALL_CORE_LIBS
        astrall::sdk
    )
endif()

add_library(astrall_core
    ${ASTRALL_CORE_SOURCES}
)

target_include_directories(astrall_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(astrall_core
    PUBLIC
        ${ASTRALL_CORE_LIBS}
)

if(ASTRALL_ENABLE_SDK)
    target_compile_definitions(astrall_core PUBLIC ASTRALL_ENABLE_SDK=1)
else()
    target_compile_definitions(astrall_core PUBLIC ASTRALL_ENABLE_SDK=0)
endif()

if(ASTRALL_BUILD_DEMOS)
    add_executable(astrall_cpp_demo examples/cpp_demo.cpp)
    target_link_libraries(astrall_cpp_demo PRIVATE astrall_core)
endif()

if(ASTRALL_BUILD_TESTS)
    enable_testing()

    add_executable(astrall_twist_mapper_test tests/test_twist_command_mapper.cpp)
    target_link_libraries(astrall_twist_mapper_test PRIVATE astrall_core)
    add_test(NAME astrall_twist_mapper_test COMMAND astrall_twist_mapper_test)
endif()
