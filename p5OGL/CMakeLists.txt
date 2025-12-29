project(p5OGL VERSION 1.0.0 LANGUAGES CXX)

set_property(CACHE APP_VERSION PROPERTY STRINGS "" v0 v1 deferred DOF gaussianBlur motionBlur)

# Decide which main file to use
if (APP_VERSION STREQUAL "")
    set(p5OGL_SOURCES main.cpp)
else()
    set(p5OGL_SOURCES main.${APP_VERSION}.cpp)
endif()

include_directories(${FREEIMAGE_INCLUDE_DIRS})

add_executable(p5OGL ${p5OGL_SOURCES})

target_include_directories(p5OGL PUBLIC ${IGL_INCLUDE})

if(WIN32)
    # Windows system OpenGL
    target_link_libraries(p5OGL IGL opengl32 GLEW::GLEW GLUT::GLUT)
else()
    # Linux / MacOS
    find_package(OpenGL REQUIRED)
    target_link_libraries(p5OGL IGL OpenGL::GL GLEW::GLEW GLUT::GLUT)
endif()

# Define absolute path to shaders folder
target_compile_definitions(p5OGL PRIVATE SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders_P5")

# Define absolute path to textures folder
target_compile_definitions(p5OGL PRIVATE TEXTURES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../img")