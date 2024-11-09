if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    file(WRITE "${CMAKE_BINARY_DIR}/set_metal_api_validation.sh"
            "export METAL_DEVICE_WRAPPER_TYPE=1\n./Cgppy")
else()
    file(WRITE "${CMAKE_BINARY_DIR}/set_metal_api_validation.sh"
            "./Cgppy")
endif()
file(CHMOD "${CMAKE_BINARY_DIR}/set_metal_api_validation.sh" PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)

set(SWIFT_BRIDGE_HEADER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Cgppy-Swift.h)
set(SOURCE_FILES_SWIFT ${CMAKE_CURRENT_SOURCE_DIR}/YExternalInterface.swift
                       ${CMAKE_CURRENT_SOURCE_DIR}/YMetalView.swift)

execute_process(COMMAND xcrun -sdk macosx swiftc -emit-objc-header -emit-objc-header-path ${SWIFT_BRIDGE_HEADER_FILE} -c ${SOURCE_FILES_SWIFT}
                RESULT_VARIABLE result
                OUTPUT_VARIABLE output)
if(NOT result EQUAL "0")
    message(FATAL_ERROR "Create swift bridge header file failed with result ${result} and output ${output}")
else ()
    message(STATUS "Create swift bridge header file success")
endif()

add_library(YSwiftLib SHARED ${SOURCE_FILES_SWIFT})

target_sources(Cgppy PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/YMetalBackend.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/YBridge.mm
)