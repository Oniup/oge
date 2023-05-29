cmake_minimum_required(VERSION 3.2)

set(EDITOR_GUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/app.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editor.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/viewport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hierarchy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/docking.cpp

    CACHE INTERNAL ""
)
