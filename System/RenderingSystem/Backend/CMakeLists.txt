add_subdirectory(Metal)
add_subdirectory(OpenGL)
add_subdirectory(Vulkan)

target_sources(Cgppy PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/YRendererBackend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/YRendererBackendManager.cpp
)