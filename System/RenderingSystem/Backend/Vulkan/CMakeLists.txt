add_subdirectory(Core)
add_subdirectory(System)

target_sources(Cgppy PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/YVulkanBackend.cpp
)