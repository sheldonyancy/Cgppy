add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Shader)

target_sources(Cgppy PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/YAssetManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/YAssets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/YAssetImporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/YDicomImporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/YFbxImporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/YObjImporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/YStlImporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/YCadImporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/YPlyImporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/YMdlaImporter.cpp
)
