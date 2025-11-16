cmake_minimum_required(VERSION 3.10)

project( g3d-practicas VERSION 0.1.0 LANGUAGES C CXX )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install/)

set(OpenGL_GL_PREFERENCE LEGACY)

include(FindFreeImage)

find_package(OpenGL REQUIRED)
find_package(FreeImage REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_subdirectory( IGL )

add_subdirectory(p1GLSL)
add_subdirectory(p2GLSL)
add_subdirectory(p3OGL)
add_subdirectory(p4OGL)