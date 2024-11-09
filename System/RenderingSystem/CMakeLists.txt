add_subdirectory(Backend)
add_subdirectory(Frontend)

target_sources(Cgppy PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/YDeveloperConsole.cpp
)
