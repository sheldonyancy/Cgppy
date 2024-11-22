cmake_minimum_required(VERSION 3.21)

project(Cgppy)

enable_language(CXX)
enable_language(C)
enable_language(Swift)
enable_language(OBJC)

add_definitions(-DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

add_executable(Cgppy
    ${CMAKE_CURRENT_SOURCE_DIR}/YEntry.cpp
)

set_target_properties(Cgppy PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS YES
    C_STANDARD 17
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS YES
    OBJC_STANDARD 11
    OBJCXX_STANDARD 14
    Swift_STANDARD 5.0
)

set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Assets)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Core)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Scene)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Components)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/EventManagement)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/System)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Assets)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shader)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Scene)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Components)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/EventManagement)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/EventManagement/Platform)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/System)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/System/PhysicsSystem)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/System/MaterialSystem)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/System/RenderingSystem)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/System/RenderingSystem/Backend)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/System/RenderingSystem/Backend/Metal)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/System/RenderingSystem/Backend/OpenGL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/System/RenderingSystem/Backend/Vulkan)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/System/RenderingSystem/Backend/Vulkan/Core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/System/RenderingSystem/Backend/Vulkan/System)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/System/RenderingSystem/Frontend)

target_compile_options(Cgppy PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-fPIC -pthread -v>
    $<$<CXX_COMPILER_ID:Clang>:-fPIC -pthread -v>
    $<$<CXX_COMPILER_ID:AppleClang>:-fPIC -pthread -v>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
