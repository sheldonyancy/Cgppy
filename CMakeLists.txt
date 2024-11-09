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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/klib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/cgltf)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spdlog/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb)

target_compile_options(Cgppy PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-fPIC -pthread -v>
    $<$<CXX_COMPILER_ID:Clang>:-fPIC -pthread -v>
    $<$<CXX_COMPILER_ID:AppleClang>:-fPIC -pthread -v>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

include(${CMAKE_CURRENT_SOURCE_DIR}/Cmake/FindPackageFallback.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(CGAL REQUIRED)
find_package(Freetype REQUIRED)
find_package(glslang REQUIRED)
find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)
find_package(shaderc REQUIRED)
find_package(jemalloc REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_library(COCOA_LIBRARY Cocoa)
find_library(METAL_LIBRARY Metal)
find_library(QUARTZ_CORE_LIBRARY QuartzCore)
find_library(FOUNDATION Foundation)

target_link_libraries(Cgppy PRIVATE GLEW::GLEW)
target_link_libraries(Cgppy PRIVATE glm::glm)
target_link_libraries(Cgppy PRIVATE Vulkan::Vulkan)
target_link_libraries(Cgppy PRIVATE Eigen3::Eigen)
target_link_libraries(Cgppy PRIVATE CGAL::CGAL)
target_link_libraries(Cgppy PRIVATE ${FREETYPE_LIBRARIES})
target_link_libraries(Cgppy PRIVATE ${GLSLANG_LIBRARIES})
target_link_libraries(Cgppy PRIVATE ${GMP_LIBRARIES})
target_link_libraries(Cgppy PRIVATE ${MPFR_LIBRARIES})
target_link_libraries(Cgppy PRIVATE ${SHADERC_LIBRARIES})
target_link_libraries(Cgppy PRIVATE ${JEMALLOC_LIBRARIES})
target_link_libraries(Cgppy PRIVATE ${COCOA_LIBRARY})
target_link_libraries(Cgppy PRIVATE ${METAL_LIBRARY})
target_link_libraries(Cgppy PRIVATE ${QUARTZ_CORE_LIBRARY})
target_link_libraries(Cgppy PRIVATE ${FOUNDATION_LIBRARY})
target_link_libraries(Cgppy PRIVATE ${OpenCV_LIBS})
target_link_libraries(Cgppy PRIVATE Boost::system Boost::filesystem)
target_link_libraries(Cgppy PRIVATE YSwiftLib)
target_link_libraries(Cgppy PRIVATE glfw)
target_link_libraries(Cgppy PRIVATE assimp)
