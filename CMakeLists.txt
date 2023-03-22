cmake_minimum_required(VERSION 3.2)
project(OGE)

add_subdirectory(ogl)

file(GLOB_RECURSE OGE_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "editor/src/*.hpp")
file(GLOB_RECURSE OGE_SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "editor/src/*.cpp")

add_executable(
    OGE

    ${OGE_HEADER_FILES}
    ${OGE_SOURCE_FILES}
)

target_link_libraries(
    OGE

    PUBLIC ogl
)

target_include_directories(
    OGE

    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/editor/src
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ogl/src
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ogl/thirdparty/entt/single_include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ogl/thirdparty/glm
)

set_target_properties(
    OGE

    PROPERTIES 
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED OFF
    CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arch"
    CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_compile_definitions(
    OGE PUBLIC 

    _CRT_NONSTDC_NO_WARNINGS
)

add_custom_command(
    TARGET OGE 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/editor/assets
    $<TARGET_FILE_DIR:OGE>/editor/assets
)

add_custom_command(
    TARGET OGE 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/ogl/assets
    $<TARGET_FILE_DIR:OGE>/ogl/assets
)

if (MSVC)
    add_custom_command(
        TARGET OGE POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        $<TARGET_FILE_DIR:ogl>
        $<TARGET_FILE_DIR:OGE>
    )

    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OGE)
endif ()
