cmake_minimum_required(VERSION 3.2)

set(EDITOR_GUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/app.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editor.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/viewport.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/viewport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hierarchy.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hierarchy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/docking.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/docking.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/console.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/console.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/assets.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/assets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/preferences.hpp

    CACHE INTERNAL ""
)
