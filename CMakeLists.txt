cmake_minimum_required(VERSION 3.2)
project(oge)

set(OGL_BUILD_SHARED TRUE CACHE INTERNAL "")
# set(OGL_BUILD_SHARED FALSE CACHE INTERNAL "")

add_subdirectory(ogl)
add_subdirectory(editor/thirdparty)

# editor source

add_subdirectory(editor/src)
add_subdirectory(editor/src/gui)
add_subdirectory(editor/src/utils)
add_subdirectory(editor/src/core)

add_executable(
    oge

    ${EDITOR_ROOT_SOURCES}
    ${EDITOR_CORE_SOURCES}
    ${EDITOR_GUI_SOURCES}
    ${EDITOR_UTILS_SOURCES}
)

target_link_libraries(
    oge

    PUBLIC ogl
    PUBLIC imgui
)

target_include_directories(
    oge

    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/editor/src
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/editor/thirdparty
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ogl/src
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ogl/thirdparty/entt/single_include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ogl/thirdparty/glm
)

set_target_properties(
    oge

    PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arch"
    CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_compile_definitions(
    oge
    PUBLIC _CRT_NONSTDC_NO_WARNINGS
)

if (OGL_BUILD_SHARED)
    target_compile_definitions(oge PUBLIC _OGL_DLL)
endif ()

add_custom_command(
    TARGET oge POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/assets
    $<TARGET_FILE_DIR:oge>/editor/assets
)

add_custom_command(
    TARGET oge POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/ogl/assets
    $<TARGET_FILE_DIR:oge>/ogl/assets
)
