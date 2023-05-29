cmake_minimum_required(VERSION 3.2)
project(OGE)

add_subdirectory(ogl)
add_subdirectory(editor/thirdparty)

# editor source

add_subdirectory(editor/src)
add_subdirectory(editor/src/gui)
add_subdirectory(editor/src/utils)
add_subdirectory(editor/src/core)

add_executable(
    OGE

    ${EDITOR_ROOT_SOURCES}
    ${EDITOR_CORE_SOURCES}
    ${EDITOR_GUI_SOURCES}
    ${EDITOR_UTILS_SOURCES}
)

target_link_libraries(
    OGE

    PUBLIC ogl
    PUBLIC imgui
)

target_include_directories(
    OGE

    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/editor/src
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/editor/thirdparty
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ogl/src
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ogl/thirdparty/entt/single_include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ogl/thirdparty/glm
)

set_target_properties(
    OGE

    PROPERTIES 
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arch"
    CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_compile_definitions(
    OGE 
    PUBLIC _CRT_NONSTDC_NO_WARNINGS
)

add_custom_command(
    TARGET OGE POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/assets
    $<TARGET_FILE_DIR:OGE>/editor/assets
)

add_custom_command(
    TARGET OGE POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/ogl/assets
    $<TARGET_FILE_DIR:OGE>/ogl/assets
)
