project(p1GLSL VERSION 1.0.0 LANGUAGES CXX)

set(p1GLSL_SOURCES 
    main.cpp
)

include_directories(${FREEIMAGE_INCLUDE_DIRS})

add_executable(p1GLSL ${p1GLSL_SOURCES})

target_include_directories(p1GLSL PUBLIC ${IGL_INCLUDE})

if(WIN32)
    # Windows system OpenGL
    target_link_libraries(p1GLSL IGL opengl32 GLEW::GLEW GLUT::GLUT assimp::assimp)
else()
    # Linux / MacOS
    target_link_libraries(p1GLSL IGL OpenGL::GL GLEW::GLEW GLUT::GLUT)
endif()

# Define absolute path to shaders folder
target_compile_definitions(p1GLSL PRIVATE SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders_P1")

# Define absolute path to textures folder
target_compile_definitions(p1GLSL PRIVATE TEXTURES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../img")



