project(p2GLSL VERSION 1.0.0 LANGUAGES CXX)

set(p2GLSL_SOURCES 
    main.cpp
)

include_directories(${FREEIMAGE_INCLUDE_DIRS})

add_executable(p2GLSL ${p2GLSL_SOURCES})

target_include_directories(p2GLSL PUBLIC ${IGL_INCLUDE})

if(WIN32)
    # Windows system OpenGL
    target_link_libraries(p2GLSL IGL opengl32 GLEW::GLEW GLUT::GLUT)
else()
    # Linux / MacOS
    find_package(OpenGL REQUIRED)
    target_link_libraries(p2GLSL IGL OpenGL::GL GLEW::GLEW GLUT::GLUT)
endif()

# Define absolute path to shaders folder
target_compile_definitions(p2GLSL PRIVATE SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders_P2")

# Define absolute path to textures folder
target_compile_definitions(p2GLSL PRIVATE TEXTURES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../img")
