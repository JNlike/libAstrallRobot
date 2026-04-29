add_library(astrall_sdk UNKNOWN IMPORTED GLOBAL)
add_library(astrall::sdk ALIAS astrall_sdk)

set(ASTRALL_SDK_ROOT ${CMAKE_CURRENT_LIST_DIR})

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
    set(ASTRALL_SDK_ARCH_DIR arm64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
    set(ASTRALL_SDK_ARCH_DIR x86-64)
else()
    message(FATAL_ERROR
        "Unsupported processor '${CMAKE_SYSTEM_PROCESSOR}'. "
        "Expected one of: x86_64, amd64, aarch64, arm64."
    )
endif()

set(ASTRALL_SDK_LIBRARY
    ${ASTRALL_SDK_ROOT}/${ASTRALL_SDK_ARCH_DIR}/libASTRALL_SDK.so
)

if(NOT EXISTS ${ASTRALL_SDK_LIBRARY})
    message(FATAL_ERROR "Astrall SDK library not found: ${ASTRALL_SDK_LIBRARY}")
endif()

set_target_properties(astrall_sdk
    PROPERTIES
        IMPORTED_LOCATION ${ASTRALL_SDK_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${ASTRALL_SDK_ROOT}/include
)

file(GLOB ASTRALL_SDK_RUNTIME_FILES
    ${ASTRALL_SDK_ROOT}/${ASTRALL_SDK_ARCH_DIR}/libASTRALL_SDK.so*
)

set(ASTRALL_SDK_RUNTIME_FILES
    ${ASTRALL_SDK_RUNTIME_FILES}
    PARENT_SCOPE
)
