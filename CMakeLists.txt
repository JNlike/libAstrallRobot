cmake_minimum_required(VERSION 3.10)

project(Astrall_RobotDog_Cpp LANGUAGES C CXX)

add_subdirectory(lib/astrall_sdk)

add_executable(sdk_demo)

target_sources(sdk_demo
    PRIVATE
        src/main.cpp
        src/keyboard.cpp
)

target_compile_features(sdk_demo PRIVATE cxx_std_17)

target_include_directories(sdk_demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(Threads REQUIRED)

target_link_libraries(sdk_demo
    PRIVATE
        astrall::sdk
        Threads::Threads
)

foreach(runtime_file IN LISTS ASTRALL_SDK_RUNTIME_FILES)
    add_custom_command(TARGET sdk_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${runtime_file}
            $<TARGET_FILE_DIR:sdk_demo>
        COMMENT "Copying Astrall SDK runtime: ${runtime_file}"
    )
endforeach()
